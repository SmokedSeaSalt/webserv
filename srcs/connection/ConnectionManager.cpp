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

    // todo error handling
    clientMap_[fd].request.newData(buf);

    return {};
}

auto ConnectionManager::handleSendingEvent(int fd) -> std::expected<void, std::string>
{
    Client client = clientMap_[fd];
    // std::string response = client.getResponse();
    // write client.Response to fd
    clientMap_[fd].response.sendDataBackToClient();

    LOG(LogLevel::kInfo, "Response sent to fd: {}.", fd);

    return {};
}

auto ConnectionManager::handleEvent(const epoll_event& epollEvent) -> std::expected<int, std::string>
{
    int      fd     = epollEvent.data.fd;
    uint32_t events = epollEvent.events;

    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    {
        clientMap_[fd].state = ClientState::Closed;
        return std::unexpected(("Connection " + std::to_string(fd) + " has been closed"));
    }

    switch (clientMap_[fd].state)
    {
    case ClientState::Receiving:
    {
        handleReceivingEvent(fd);
        if (clientMap_[fd].request.getState() != RequestState::KDone)
            break;
        LOG(LogLevel::kInfo, "Packet received from fd: {}.", fd);
        clientMap_[fd].response = execution_.execute(clientMap_[fd].request.getMessage());
        clientMap_[fd].state    = ClientState::Processing;
        [[fallthrough]];
    }
    case ClientState::Processing:
    {
        if (clientMap_[fd].response.isReadyToSend())
            clientMap_[fd].state = ClientState::Sending;
        else
            break;
    }
    case ClientState::Sending:
        handleSendingEvent(fd);
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
        clientMap_[listenSocket].state = ClientState::Closed;
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