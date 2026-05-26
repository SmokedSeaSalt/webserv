#include "Server.hpp"
#include "ConnectionManager.hpp"
#include "Execution.hpp"
#include "connection.hpp"
#include "logging.hpp"
#include "signals.hpp"
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

Server::Server() {}

Server::~Server() {}

auto Server::getListenServerAddress(std::string ip, int port)
    -> std::expected<sockaddr_in, std::string>
{
    sockaddr_in listenServerAddress;

    bzero(&listenServerAddress, sizeof(listenServerAddress));
    listenServerAddress.sin_family = AF_INET;
    listenServerAddress.sin_port   = htons(port);
    if (ip.empty())
        listenServerAddress.sin_addr.s_addr = INADDR_ANY;
    else if (inet_pton(AF_INET, ip.c_str(), &listenServerAddress.sin_addr) != 1)
    {
        perror("inet_pton");
        return std::unexpected("inet_pton failed");
    }
    return listenServerAddress;
}

auto Server::setupListenSocket(std::string ip, int port) -> std::expected<int, std::string>
{
    if (ip == "localhost")
        ip = "127.0.0.1";
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

    // to ensure you can reuse sockets after restarting the server
    int opt = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt(SO_REUSEADDR)");
        close(listenSocket);
        return std::unexpected("setsockopt() failed");
    }

    if (bind(listenSocket, reinterpret_cast<struct sockaddr*>(&listenServerAddress.value()), sizeof(listenServerAddress)) == -1)
    {
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
    for (const auto& [fd, ipPortPair] : listenSocketFdToIpPortPair_)
    {
        epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
    }
}

auto Server::setupListenSockets() -> std::expected<void, std::string>
{

    for (Config::ServerBlock serverBlock : Config::config.serverBlocks)
    {
        auto fd = setupListenSocket(serverBlock.ip, serverBlock.port);
        if (!fd.has_value())
        {
            closeListenSockets();
            return std::unexpected(fd.error());
        }
        listenSocketFdToIpPortPair_[fd.value()] = make_tuple(serverBlock.ip, serverBlock.port);

        ev_.events  = EPOLLIN;
        ev_.data.fd = fd.value();
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd.value(), &ev_) == -1)
        {
            closeListenSockets();
            return std::unexpected("epoll_ctl() failed");
        }
        LOG(LogLevel::kInfo, "Listen socket ip={} port={} opened at: {}.", serverBlock.ip, serverBlock.port, fd.value());
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

    ConnectionManager::setEpollfd(epollfd_);
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
        int epollTimeoutMs = 1000;
        nfds               = epoll_wait(epollfd_, events_, MAX_EVENTS, epollTimeoutMs);
        if (nfds == -1)
        {
            LOG(LogLevel::kDebug, "Epoll wait returned -1");
            break;
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (listenSocketFdToIpPortPair_.contains(events_[n].data.fd))
            {
                ConnectionManager::createConnection(events_[n], listenSocketFdToIpPortPair_[events_[n].data.fd]);
            }
            else
            {
                ConnectionManager::handleEvent(events_[n]);
            }
        }
        ConnectionManager::processTimeouts();
        if (Signals::shouldShutdown == true)
        {
            LOG(LogLevel::kDebug, "Terminating connection loop.");
            break;
        }
    }
    serverCleanup();
    return {};
}

auto Server::serverCleanup() -> void
{
    ConnectionManager::connectionManagerCleanup();
    closeListenSockets();
    close(epollfd_);
}

// Server
// |-- vector<ListenSocket>
// |   |-- struct config;
// |   |-- fdint fd;
// |-- map<fd, client>
// |   |-- ListenSocket& parentSocket; // same as listenSocket::config
// |   |-- enum class ClientState;
