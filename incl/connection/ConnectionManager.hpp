#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include "Execution.hpp"
#include "connection.hpp"
#include <netinet/in.h>
#include <sys/epoll.h>

enum class HandleEventResult
{
    kSuccess,
    kError,
};

class ConnectionManager
{
    public:
        ConnectionManager(int epollfd);

        auto handleEvent(const epoll_event& epollEvent) -> HandleEventResult;

        auto createConnection(const epoll_event& epollEvent, int listenSocketPort) -> std::expected<void, std::string>;

    private:
        std::map<int, Client> clientMap_;
        Execution             execution_;
        struct epoll_event    ev_;
        int                   epollfd_;

        auto handleReceivingEvent(int fd) -> std::expected<void, std::string>;
        auto handleSendingEvent(int fd) -> std::expected<void, std::string>;

        auto logNewConnection(int connectionSocket, sockaddr_storage clientAddress, socklen_t addressLen) -> void;
};

#endif // CONNECTIONMANAGER_HPP