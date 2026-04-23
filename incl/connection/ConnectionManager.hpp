#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include "Execution.hpp"
#include "connection.hpp"
#include <sys/epoll.h>
#include <netinet/in.h>


class ConnectionManager
{
    public:
        ConnectionManager(Config config, int epollfd);

        auto handleEvent(int fd) -> std::expected<int, std::string>;
        auto createConnection(int listenSocket) -> std::expected<void, std::string>;

    private:
        std::map<int, Client> clientMap_;
        Execution             execution_;
        struct epoll_event    ev_;
        int                   epollfd_;
        Config                config_;

        auto handleReceivingEvent(int fd) -> std::expected<void, std::string>;
        auto handleSendingEvent(int fd) -> std::expected<void, std::string>;

        auto logNewConnection(int connectionSocket, sockaddr_storage clientAddress, socklen_t addressLen) -> void;

};

#endif // CONNECTIONMANAGER_HPP