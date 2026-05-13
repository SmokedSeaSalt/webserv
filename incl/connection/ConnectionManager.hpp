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
        static auto addCGIConnection(int cgiFd, std::shared_ptr<Client> client)  -> std::expected<void, std::string>;

        static auto closeConnection(int fd) -> void;
        static auto handleReceivingEvent(int fd) -> std::tuple<std::string, ssize_t>;
        static auto handleSendingEvent(int fd, std::string data) -> ssize_t;

        static auto getClient(int fd) -> std::shared_ptr<Client>;

    private:
        static std::map<int, std::shared_ptr<Client>> clientMap_;
        static int                   epollfd_;

        // auto logNewConnection(int connectionSocket, sockaddr_storage clientAddress, socklen_t addressLen) -> void;
};

#endif // CONNECTIONMANAGER_HPP