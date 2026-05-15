#include "ConnectionManager.hpp"
#include "Client.hpp"
#include "logging.hpp"
#include <arpa/inet.h> // for client logging
#include <netdb.h>     // for client logging
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Static member definitions
std::map<int, std::shared_ptr<Client>> ConnectionManager::clientMap_;
int                                    ConnectionManager::epollfd_;

auto ConnectionManager::setEpollfd(int epollfd) -> void
{
    epollfd_ = epollfd;
}

/// @brief receives data from the fd
/// @param fd
/// @return void or error string if recv() fails or client has closed the connection.
auto ConnectionManager::handleReceivingEvent(int fd) -> std::tuple<std::string, ssize_t>
{
    std::string buf;
    buf.resize(BUFFER_SIZE);
    ssize_t numBytes = recv(fd, buf.data(), BUFFER_SIZE, 0);
    // if (numBytes < 0)
    //     return std::unexpected("recv failed");
    // if (numBytes == 0)
    //     return std::unexpected("Client " + std::to_string(fd) + " has disconnected. recv returned 0.");

    // todo error handling
    return {buf, numBytes};
}

auto ConnectionManager::handleSendingEvent(int fd, std::string data) -> ssize_t
{
    ssize_t bytesSent = send(fd, data.c_str(), data.size(), 0);
    // LOG(LogLevel::kInfo, "remaining packet len {}.", response.getRemainingPacketLen());

    return bytesSent;
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
    Client& client = *it->second;

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

auto ConnectionManager::closeConnection(int fd) -> void
{
    epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL);
    if (close(fd) == -1)
        LOG(LogLevel::kDebug, "failed to close client fd: {}", fd);
    else
    {
        LOG(LogLevel::kDebug, "Closed client fd: {}. Erasing from clientMap_", fd);
        clientMap_.erase(fd);
    }
}

auto ConnectionManager::addCGIConnection(int cgiFd, std::shared_ptr<Client> client) -> std::expected<void, std::string>
{
    auto setNonBlockingRes = setNonBlocking(cgiFd);
    if (!setNonBlockingRes.has_value())
        return std::unexpected(setNonBlockingRes.error());

    struct epoll_event ev_;
    ev_.events  = EPOLLIN | EPOLLRDHUP | EPOLLOUT;
    ev_.data.fd = cgiFd;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, cgiFd, &ev_) == -1)
    {
        perror("epoll_ctl: connectionSocket");
        return std::unexpected("epoll_ctl failed");
    }
    clientMap_.emplace(cgiFd, client);
    return {};
}

auto ConnectionManager::createConnection(const epoll_event& epollEvent, std::tuple<std::string, int> ipPortPair) -> std::expected<void, std::string>
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

    clientMap_.emplace(connectionSocket, std::make_shared<Client>(connectionSocket, listenSocket, ipPortPair, std::string(service), std::string(host)));

    return {};
}

auto ConnectionManager::getClient(int fd) -> std::shared_ptr<Client>
{
    auto it = clientMap_.find(fd);
    if (it == clientMap_.end())
        return nullptr;
    return it->second;
}

auto ConnectionManager::connectionManagerCleanup() -> void
{
    int cgiPID;
    for (auto& [fd, client] : clientMap_)
    {
        cgiPID = client->getCgiPID();
        if (cgiPID > 0)
        {
            kill(cgiPID, SIGTERM); // todo SIGTERM or SIGKILL
            waitpid(cgiPID, NULL, 0);
        }
        closeConnection(fd);
    }
}

auto ConnectionManager::processTimeouts() -> void
{
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::vector<int>                      fdsToClose;
    for (const auto& [fd, client] : clientMap_)
    {
        auto idleSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - client->getLastActivity()).count();
        if (idleSeconds > clientTimeoutSeconds_)
            fdsToClose.push_back(fd);
    }
    for (int fd : fdsToClose)
    {
        LOG(LogLevel::kDebug, "Client fd: {} timed out", fd);
        closeConnection(fd);
    }
}
