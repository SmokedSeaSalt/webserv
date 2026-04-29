#include "ConnectionManager.hpp"
#include "Client.hpp"
#include "logging.hpp"
#include <arpa/inet.h> // for client logging
#include <netdb.h>     // for client logging
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>

// Static member definitions
std::map<int, Client> ConnectionManager::clientMap_;
int                   ConnectionManager::epollfd_;

auto ConnectionManager::setEpollfd(int epollfd) -> void
{
    epollfd_ = epollfd;
}

/// @brief receives data from the fd
/// @param fd
/// @return void or error string if recv() fails or client has closed the connection.
auto ConnectionManager::handleReceivingEvent(int fd) -> std::expected<std::string, std::string>
{
    std::string buf;
    buf.resize(BUFFER_SIZE);
    ssize_t numBytes = recv(fd, buf.data(), BUFFER_SIZE, 0);
    if (numBytes < 0)
        return std::unexpected("recv failed");
    if (numBytes == 0)
        return std::unexpected("Client " + std::to_string(fd) + " has disconnected. recv returned 0.");

    // todo error handling
    return buf;
}

auto ConnectionManager::handleSendingEvent(int fd, HTTPResponse& response) -> std::expected<void, std::string>
{

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
            client.setState(ClientState::Closed);
            clientMap_.erase(fd);
        }
        return HandleEventResult::kError;
    }

    return client.handleEvent(epollEvent);
}

// auto ConnectionManager::logNewConnection(int connectionSocket, sockaddr_storage clientAddress, socklen_t addressLen) -> void
// {
//     char host[NI_MAXHOST];
//     char service[NI_MAXSERV];

//     int rc = getnameinfo(reinterpret_cast<sockaddr*>(&clientAddress), addressLen,
//                          host, sizeof(host), service, sizeof(service),
//                          NI_NUMERICHOST | NI_NUMERICSERV);

// }

auto ConnectionManager::eraseClient(int fd) -> void
{
    if (close(fd) == -1)
        LOG(LogLevel::kDebug, "failed to close client fd: {}", fd);
    else
    {
        LOG(LogLevel::kDebug, "Closed client fd: {}. Erasing from clientMap_", fd);
        clientMap_.erase(fd);
    }
}

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

    struct epoll_event ev_;
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
    {
        LOG(LogLevel::kInfo, "Client connected: fd={}", connectionSocket);
        return std::unexpected("getnameinfo could not get all paramaters");
    }

    clientMap_.emplace(connectionSocket, Client(connectionSocket, listenSocketPort, std::string(service), std::string(host)));

    return {};
}