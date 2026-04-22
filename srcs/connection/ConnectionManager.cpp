#include "ConnectionManager.hpp"
#include <netinet/in.h>
#include <sys/epoll.h>
#include "logging.hpp"

ConnectionManager::ConnectionManager(Config config, int epollfd) : execution_(config), epollfd_(epollfd) , config_(config) {}

auto ConnectionManager::handleReceivingEvent(int fd) -> std::expected<void, std::string>
{
    std::string buf;
    buf.resize(BUFFER_SIZE);
    ssize_t numBytes = recv(fd, buf.data(), BUFFER_SIZE - 1, 0);
    if (numBytes < 0)
        return std::unexpected("recv failed");

    buf[numBytes] = '\0';
    // todo error handling
    clientMap_[fd].request.newData(buf);

    return {};
}

auto ConnectionManager::handleSendingEvent(int fd) -> std::expected<void, std::string>
{
    Client client = clientMap_[fd];
    // std::string response = client.getResponse();
    // write client.Response to fd
    return {};
}

auto ConnectionManager::handleEvent(int fd) -> std::expected<int, std::string>
{

    switch (clientMap_[fd].state)
    {
    case ClientState::Receiving:
        handleReceivingEvent(fd);
        if (clientMap_[fd].request.getState() != RequestState::KDone)
            break;
        // LOG(LogLevel::kDebug, "{}", clientMap_[fd].request.getMessage().body);
        execution_.execute(clientMap_[fd].request.getMessage());
        clientMap_[fd].state = ClientState::Processing;
    case ClientState::Processing:
        break;
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

auto ConnectionManager::createConnection(int listenSocket) -> std::expected<void, std::string>
{
    int connectionSocket;

    // todo: can provide more args to log info on clients
    connectionSocket = accept(listenSocket, nullptr, nullptr);
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
    return {};
}