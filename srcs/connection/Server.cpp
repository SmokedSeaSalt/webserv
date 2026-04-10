#include "Server.hpp"
#include "connection.hpp"
#include <arpa/inet.h>
#include <expected>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(Config config) : config_(config) {}

Server::~Server() {}

auto Server::setNonBlocking(int socketfd) -> std::expected<void, std::string>
{
    if (fcntl(socketfd, F_SETFL, O_NONBLOCK) == -1)
    {
        perror("fcntl");
        return std::unexpected("fcntl failed");
    }
    return {};
}

auto Server::handleReceivingEvent(int fd) -> std::expected<void, std::string>
{
    std::string buf;
    buf.resize(BUFFER_SIZE);
    ssize_t numBytes = recv(fd, buf.data(), BUFFER_SIZE - 1, 0);
    if (numBytes < 0)
        return std::unexpected("recv failed");

    buf[numBytes] = '\0';
    clientMap_[fd].request.newData(buf);

    return {};
}

auto Server::handleSendingEvent(int fd) -> std::expected<void, std::string>
{
    Client client = clientMap_[fd];
    // std::string response = client.getResponse();
    // write client.Response to fd
    return {};
}

auto Server::handleEvent(int fd) -> std::expected<int, std::string>
{

    switch (clientMap_[fd].state)
    {
    case ClientState::Receiving:
        handleReceivingEvent(fd);
        break;
    case ClientState::Received:
        execution.execute(clientMap_[fd].request);
        clientMap_[fd].state = ClientState::Processing;
        break;
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

auto Server::createConnection(int listenSocket) -> std::expected<void, std::string>
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

auto Server::getListenServerAddress(std::string ip, int port)
    -> std::expected<sockaddr_in, std::string>
{
    sockaddr_in listenServerAddress;

    bzero(&listenServerAddress, sizeof(listenServerAddress));
    listenServerAddress.sin_family = AF_INET;
    listenServerAddress.sin_port   = htons(port);
    if (ip.empty())
        listenServerAddress.sin_addr.s_addr = INADDR_ANY;
    else if (inet_pton(AF_INET, ip.c_str(), &listenServerAddress.sin_addr) !=
             1) // todo: check two different fail cases?
    {
        perror("inet_pton");
        return std::unexpected("inet_pton failed");
    }
    return listenServerAddress;
}

auto Server::setupListenSocket(std::string ip, int port) -> std::expected<int, std::string>
{
    // Create socket with IPv4 and TCP
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1)
    {
        perror("socket");
        return std::unexpected("socket() failed");
    }

    auto listenServerAddress = getListenServerAddress(ip, port);
    if (!listenServerAddress.has_value())
        return std::unexpected(listenServerAddress.error());

    if (bind(listenSocket, (struct sockaddr*)&listenServerAddress, sizeof(listenServerAddress)) ==
        -1)
    {
        printf("listenSocket: %d", listenSocket);
        perror("bind");
        return std::unexpected("bind() failed");
    }

    if (listen(listenSocket, SOMAXCONN) == -1)
    {
        perror("listen");
        return std::unexpected("listen() failed");
    }

    auto setNonBlockingRes = setNonBlocking(listenSocket);
    if (!setNonBlockingRes.has_value())
        return std::unexpected(setNonBlockingRes.error());

    return listenSocket;
}

auto Server::closeListenSockets() -> void
{
    for (int fd : listenSockets_)
        close(fd);
}

auto Server::setupListenSockets() -> std::expected<void, std::string>
{

    for (ServerBlock serverBlock : config_.serverBlocks)
    {
        auto fd = setupListenSocket(serverBlock.ip, serverBlock.port);
        if (!fd.has_value())
        {
            closeListenSockets();
            return std::unexpected(fd.error());
        }
        listenSockets_.insert(fd.value());

        ev_.events  = EPOLLIN;
        ev_.data.fd = fd.value();
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd.value(), &ev_) == -1)
        {
            closeListenSockets();
            return std::unexpected("epoll_ctl() failed");
        }
    }
    return {};
}

auto Server::setup() -> std::expected<void, std::string>
{
    epollfd_ = epoll_create(1);
    if (epollfd_ == -1)
    {
        perror("epoll_create");
        return std::unexpected("epoll_create() failed");
    }
    ServerBlock test{}; // FOR NOW HARDCODED TEST
    test.ip   = "";
    test.port = 8080;
    config_.serverBlocks.push_back(test); // FOR NOW HARDCODED TEST
    auto ret = setupListenSockets();
    if (!ret.has_value())
        return std::unexpected(ret.error());
    return {};
}

auto Server::connection_loop() -> std::expected<void, std::string>
{
    int nfds;

    while (true)
    {
        nfds = epoll_wait(epollfd_, events_, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            return std::unexpected("epoll_wait() failed");
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (listenSockets_.contains(events_[n].data.fd))
                createConnection(events_[n].data.fd);
            else
                handleEvent(events_[n].data.fd);
        }
    }
    return {};
}

// Getters
auto Server::getListenSockets() -> std::set<int>
{
    return listenSockets_;
}

// Server
// |-- vector<ListenSocket>
// |   |-- struct config;
// |   |-- fdint fd;
// |-- map<fd, client>
// |   |-- ListenSocket& parentSocket; // same as listenSocket::config
// |   |-- enum class ClientState;
