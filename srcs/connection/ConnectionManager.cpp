#include "ConnectionManager.hpp"
#include "logging.hpp"
#include <arpa/inet.h> // for client logging
#include <netdb.h>     // for client logging
#include <netinet/in.h>
#include <sys/epoll.h>

ConnectionManager::ConnectionManager(Config config, int epollfd) : execution_(config), epollfd_(epollfd), config_(config) {}

auto ConnectionManager::handleReceivingEvent(int fd) -> std::expected<void, std::string>
{
    std::string buf;
    buf.resize(BUFFER_SIZE);
    ssize_t numBytes = recv(fd, buf.data(), BUFFER_SIZE, 0);
    if (numBytes < 0)
        return std::unexpected("recv failed");
    if (numBytes == 0)
        return std::unexpected("Client " + std::to_string(fd) + " has disconnected. recv returned 0.");


    // todo error handling

    auto it = clientMap_.find(fd);
    if (it == clientMap_.end())
        return std::unexpected("unknown client fd: " + std::to_string(fd));
    Client& client = it->second;
    client.request.newData(buf);

    return {};
}

auto ConnectionManager::handleSendingEvent(int fd) -> std::expected<void, std::string>
{
    // std::string response = client.getResponse();
    // write client.Response to fd

    auto it = clientMap_.find(fd);
    if (it == clientMap_.end())
        return std::unexpected("unknown client fd: " + std::to_string(fd));
    Client& client = it->second;
    HTTPResponse response = client.response;

    SendState sendState = response.getSendState();

    if (sendState == SendState::kReady) // first send, needs to init the packet
    {
        std::string packet = response.createPacket();
        response.setPacket(packet);
    }

    if (sendState == SendState::kReady || sendState == SendState::kSending)
    {
        std::string buf = response.getRemainingPacket();

        ssize_t bytesSent = send(fd, buf.c_str(), response.getRemainingPacketLen(), 0);
        if (bytesSent < 0)
        {
            response.setSendState(SendState::kSending);
            return std::unexpected("send failed");
            // todo handle error
        }
        if (bytesSent == 0)
        {
            response.setSendState(SendState::kSending);
            
        }

        response.incrementTotalBytesSent(bytesSent);
        response.setSendState(SendState::kSending);
    }

    LOG(LogLevel::kInfo, "Response sent to fd: {}.", fd);

    return {};
}

auto ConnectionManager::handleEvent(const epoll_event& epollEvent) -> std::expected<int, std::string>
{
    int      fd     = epollEvent.data.fd;
    uint32_t events = epollEvent.events;

    auto it = clientMap_.find(fd);
    if (it == clientMap_.end())
        return std::unexpected("unknown client fd: " + std::to_string(fd));
    Client& client = it->second;

    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    {
        client.state = ClientState::Closed;
        return std::unexpected(("Connection " + std::to_string(fd) + " has been closed"));
    }

    switch (client.state)
    {
    case ClientState::Receiving:
    {
        if (!(events & EPOLLIN))
            break;
        auto handleReceivingEventResult = handleReceivingEvent(fd);
        if (!handleReceivingEventResult.has_value())
            return std::unexpected(handleReceivingEventResult.error());
        if (client.request.getState() != RequestState::KDone)
            break;
        LOG(LogLevel::kInfo, "Packet received from fd: {}.", fd);
        client.response = execution_.execute(client.request.getMessage());
        [[fallthrough]];
    }
    case ClientState::Processing:
    {
        if (client.response.getSendState() == SendState::kReady)
            client.state = ClientState::Sending;
        else
        {
            client.state = ClientState::Processing;
            break;
        }
    }
    case ClientState::Sending:
    {
        if (!(events & EPOLLOUT))
            break;
        handleSendingEvent(fd);
    }
    break;
    case ClientState::Closed:
        break;
    case ClientState::Error:
        break;
    }
    return 0; // todo: what to return here?
}

auto ConnectionManager::logNewConnection(int connectionSocket, sockaddr_storage clientAddress, socklen_t addressLen) -> void
{
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int rc = getnameinfo(reinterpret_cast<sockaddr*>(&clientAddress), addressLen,
                         host, sizeof(host), service, sizeof(service),
                         NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc == 0)
    {
        LOG(LogLevel::kInfo, "Client connected: ip={} port={} fd={}", host, service, connectionSocket);
    }
    else
    {
        LOG(LogLevel::kInfo, "Client connected: fd={}", connectionSocket);
    }
}

auto ConnectionManager::createConnection(const epoll_event& epollEvent) -> std::expected<void, std::string>
{
    int              connectionSocket;
    sockaddr_storage clientAddress{};
    socklen_t        addressLen   = sizeof(clientAddress);
    int              listenSocket = epollEvent.data.fd;
    uint32_t         events       = epollEvent.events;

    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    {
        return std::unexpected(("Listen socket " + std::to_string(listenSocket) + " has been closed"));
    }

    // todo: can provide more args to log info on clients
    connectionSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddress), &addressLen);
    if (connectionSocket == -1)
    {
        perror("accept");
        return std::unexpected("accept failed");
    }

    auto setNonBlockingRes = setNonBlocking(connectionSocket);
    if (!setNonBlockingRes.has_value())
        return std::unexpected(setNonBlockingRes.error());

    ev_.events  = EPOLLIN | EPOLLRDHUP;
    ev_.data.fd = connectionSocket;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, connectionSocket, &ev_) == -1)
    {
        perror("epoll_ctl: connectionSocket");
        return std::unexpected("epoll_ctl failed");
    }
    clientMap_.emplace(connectionSocket, Client{.socketfd = connectionSocket,
                                                .state    = ClientState::Receiving,
                                                .request  = HTTPRequest{},
                                                .response = HTTPResponse{},
                                                .error    = ErrorType::None});
    logNewConnection(connectionSocket, clientAddress, addressLen);
    return {};
}