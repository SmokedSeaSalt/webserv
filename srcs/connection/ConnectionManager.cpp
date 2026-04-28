#include "ConnectionManager.hpp"
#include "logging.hpp"
#include <arpa/inet.h> // for client logging
#include <netdb.h>     // for client logging
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>

ConnectionManager::ConnectionManager(int epollfd) : execution_(), epollfd_(epollfd) {}

/// @brief receives data from the fd
/// @param fd
/// @return void or error string if recv() fails or client has closed the connection.
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
    Client&       client   = it->second;
    HTTPResponse& response = client.response;

    if (response.getSendState() == SendState::kReady) // first send, needs to init the packet
    {
        std::string packet = response.createPacket();
        LOG(LogLevel::kDebug, "Sending packet to fd:{} with content:\n{}\n", fd, packet);

        response.setPacket(packet);
        response.setSendState(SendState::kSending);
    }

    if (response.getSendState() == SendState::kSending)
    {
        std::string buf = response.getRemainingPacket();

        ssize_t bytesSent = send(fd, buf.c_str(), response.getRemainingPacketLen(), 0);
        if (bytesSent < 0)
        {
            response.setSendState(SendState::kFailed);
            return std::unexpected("send failed");
            // todo handle error
        }
        response.incrementTotalBytesSent(bytesSent);
        if (bytesSent == 0 || response.getRemainingPacketLen() == 0)
        {
            LOG(LogLevel::kInfo, "Request finished sending", fd);
            response.setSendState(SendState::kDone);
            return {};
        }
    }

    // LOG(LogLevel::kInfo, "remaining packet len {}.", response.getRemainingPacketLen());

    return {};
}

auto ConnectionManager::handleEvent(const epoll_event& epollEvent) -> HandleEventResult
{
    int      fd     = epollEvent.data.fd;
    uint32_t events = epollEvent.events;

    auto it = clientMap_.find(fd);
    if (it == clientMap_.end())
    {
        LOG(LogLevel::kDebug, "unknown client fd: {}", std::to_string(fd));
        return HandleEventResult::kError;
    }
    Client& client = it->second;

    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    {
        if (close(fd) == -1)
            LOG(LogLevel::kDebug, "failed to close client fd: {}", std::to_string(fd));
        else
        {
            LOG(LogLevel::kDebug, "Closed client fd: {}", std::to_string(fd));
            client.state = ClientState::Closed;
            clientMap_.erase(fd);
        }
        return HandleEventResult::kError;
    }

    switch (client.state)
    {
    case ClientState::Receiving:
    {
        if (!(events & EPOLLIN))
            break;
        auto handleReceivingEventResult = handleReceivingEvent(fd);
        if (!handleReceivingEventResult.has_value())
        {
            LOG(LogLevel::kDebug, "recv() returned <= 0 at fd: {}, closing connection.", std::to_string(fd));
            if (close(fd) == -1)
                LOG(LogLevel::kDebug, "failed to close client fd: {}", std::to_string(fd));
            else
                client.state = ClientState::Closed;
            return HandleEventResult::kError;
        }
        if (client.request.getState() != RequestState::KDone)
            break;
        LOG(LogLevel::kInfo, "Packet received from fd:{} with content:\n{}\n", fd, getHTTPMessageString(client.request.getMessage()));
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
        if (client.response.getSendState() == SendState::kDone)
            client.state = ClientState::Sent;
        break;
    }
    case ClientState::Sent:
    {
        if (client.response.getKeepAlive() == false)
        {
            if (close(fd) == -1)
                LOG(LogLevel::kDebug, "failed to close client fd: {}", std::to_string(fd));
            else
            {
                LOG(LogLevel::kDebug, "Closed client fd: {}. Erasing from clientMap_", std::to_string(fd));
                clientMap_.erase(fd);
            }
        }
        else
        {
            client.request  = {};
            client.response = {};
            client.state    = ClientState::Receiving;
            LOG(LogLevel::kDebug, "Client socket at fd: {} being kept alive", std::to_string(fd));
        }
        break;
    }
    case ClientState::Closed:
        break;
    case ClientState::Error:
        break;
    }
    return HandleEventResult::kSuccess; // todo: what to return here?
}

// auto ConnectionManager::logNewConnection(int connectionSocket, sockaddr_storage clientAddress, socklen_t addressLen) -> void
// {
//     char host[NI_MAXHOST];
//     char service[NI_MAXSERV];

//     int rc = getnameinfo(reinterpret_cast<sockaddr*>(&clientAddress), addressLen,
//                          host, sizeof(host), service, sizeof(service),
//                          NI_NUMERICHOST | NI_NUMERICSERV);

// }

auto ConnectionManager::createConnection(const epoll_event& epollEvent, int listenSocketPort) -> std::expected<void, std::string>
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

    ev_.events  = EPOLLIN | EPOLLRDHUP | EPOLLOUT;
    ev_.data.fd = connectionSocket;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, connectionSocket, &ev_) == -1)
    {
        perror("epoll_ctl: connectionSocket");
        return std::unexpected("epoll_ctl failed");
    }

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    int  rc = getnameinfo(reinterpret_cast<sockaddr*>(&clientAddress), addressLen,
                          host, sizeof(host), service, sizeof(service),
                          NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc == 0)
        LOG(LogLevel::kInfo, "Client connected: ip={} port={} fd={}", host, service, connectionSocket);
    else
        LOG(LogLevel::kInfo, "Client connected: fd={}", connectionSocket);

    clientMap_.emplace(connectionSocket, Client{.socketfd         = connectionSocket,
                                                .listenSocketPort = listenSocketPort,
                                                .service          = std::string(service),
                                                .host             = std::string(host),
                                                .state            = ClientState::Receiving,
                                                .request          = HTTPRequest{},
                                                .response         = HTTPResponse{},
                                                .error            = ErrorType::None});

    return {};
}