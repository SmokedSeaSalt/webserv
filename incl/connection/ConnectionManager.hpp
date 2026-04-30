#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include "Client.hpp"
#include "HTTPResponse.hpp"
#include <expected>
#include <map>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>

enum class HandleEventResult
{
    kSuccess,
    kError,
};

class ConnectionManager
{
    public:
        static auto setEpollfd(int epollfd) -> void;

        static auto handleEvent(const epoll_event& epollEvent) -> HandleEventResult;
        static auto createConnection(const epoll_event& epollEvent, std::tuple<std::string, int>) -> std::expected<void, std::string>;
        static auto addCGIConnection(int cgiFd, Client& client)  -> std::expected<void, std::string>;

        static auto eraseClient(int fd) -> void;
        static auto handleReceivingEvent(int fd) -> std::expected<std::string, std::string>;
        static auto handleSendingEvent(int fd, HTTPResponse& response) -> std::expected<void, std::string>;

    private:
        static std::map<int, Client> clientMap_;
        static int                   epollfd_;

        // auto logNewConnection(int connectionSocket, sockaddr_storage clientAddress, socklen_t addressLen) -> void;
};

#endif // CONNECTIONMANAGER_HPP