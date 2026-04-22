#ifndef SERVER_HPP
#define SERVER_HPP

#include "ConnectionManager.hpp"
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
        Server(Config config);
        ~Server();

        auto setup() -> std::expected<void, std::string>;
        auto connection_loop() -> std::expected<void, std::string>;

        // getters
        auto getListenSockets() -> std::set<int>;

    private:
        // std::map<int, Client> clientMap_;
        struct epoll_event                 ev_;
        struct epoll_event                 events_[MAX_EVENTS];
        struct Config                      config_;
        std::unique_ptr<ConnectionManager> connectionManager_;
        std::set<int>                      listenSockets_;
        int                                epollfd_;
        // Execution             execution_;

        // setup() helpers
        auto setupListenSocket(std::string ip, int port) -> std::expected<int, std::string>;
        auto setupListenSockets() -> std::expected<void, std::string>;

        auto getListenServerAddress(std::string ip, int port)
            -> std::expected<sockaddr_in, std::string>;

        // connectionLoop helpers
        // these have been moved to ConnectionManager. delete later.
        // auto handleEvent(int fd) -> std::expected<int, std::string>;
        // auto handleReceivingEvent(int fd) -> std::expected<void, std::string>;
        // auto handleSendingEvent(int fd) -> std::expected<void, std::string>;
        // auto createConnection(int listenSocket) -> std::expected<void, std::string>;

        // cleanup
        auto closeListenSockets() -> void;
};

#endif // SERVER_HPP