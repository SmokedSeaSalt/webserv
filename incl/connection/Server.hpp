#ifndef SERVER_HPP
#define SERVER_HPP

#include "configParsing.hpp"
#include "connection.hpp"
#include <expected>
#include <netinet/in.h>
#include <set>
#include <string>
#include <sys/epoll.h>
#include <vector>

class Server
{
    public:
        Server(Config config);
        ~Server();

        auto setup() -> std::expected<void, std::string>;
        auto connection_loop() -> std::expected<void, std::string>;

        // getters
        auto getListenSockets() -> std::set<int>;

    private:
        struct epoll_event    ev_;
        struct epoll_event    events_[MAX_EVENTS];
        struct Config         config_;
        std::map<int, Client> clientMap_;
        std::set<int>         listenSockets_;
        int                   epollfd_;

        // setup() helpers
        auto setupListenSocket(std::string ip, int port) -> std::expected<int, std::string>;
        auto setupListenSockets() -> std::expected<void, std::string>;
        auto setNonBlocking(int connSock) -> std::expected<void, std::string>;

        auto getListenServerAddress(std::string ip, int port)
            -> std::expected<sockaddr_in, std::string>;

        // connectionLoop helpers
        auto handleEvent(int fd) -> std::expected<int, std::string>;
        auto handleReceivingEvent(int fd) -> std::expected<void, std::string>;
        auto handleSendingEvent(int fd) -> std::expected<void, std::string>;
        auto createConnection(int listenSocket) -> std::expected<void, std::string>;

        // cleanup
        auto closeListenSockets() -> void;
};

#endif // SERVER_HPP