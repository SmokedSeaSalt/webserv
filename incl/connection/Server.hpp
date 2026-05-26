#ifndef SERVER_HPP
#define SERVER_HPP

#include "Execution.hpp"
#include "configParsing.hpp"
#include "connection.hpp"
#include <expected>
#include <memory>
#include <netinet/in.h>
#include <set>
#include <string>
#include <sys/epoll.h>
#include <vector>

class Server
{
    public:
        Server();
        ~Server();

        auto setup() -> std::expected<void, std::string>;
        auto connection_loop() -> std::expected<void, std::string>;



    private:
        struct epoll_event                          ev_;
        struct epoll_event                          events_[MAX_EVENTS];
        std::map<int, std::tuple<std::string, int>> listenSocketFdToIpPortPair_;
        int                                         epollfd_;

        // setup() helpers
        auto setupListenSocket(std::string ip, int port) -> std::expected<int, std::string>;
        auto setupListenSockets() -> std::expected<void, std::string>;

        auto getListenServerAddress(std::string ip, int port)
            -> std::expected<sockaddr_in, std::string>;

        // cleanup
        auto closeListenSockets() -> void;
        auto serverCleanup() -> void;
};

#endif // SERVER_HPP