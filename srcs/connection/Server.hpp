#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "connection.hpp"
#include <vector>
#include <expected>
#include <set>

#include "configParsing.hpp"

class Server
{
public:
    Server(std::string configFile);
    ~Server();

    auto setup() -> std::expected<void, std::string>;
    auto connection_loop() -> std::expected<void, std::string>;


private:
    struct epoll_event      ev_;
    struct epoll_event      events_[MAX_EVENTS];
    struct Config           config_;
    std::map<int, Client>   clientMap_;
    std::set<int>           listenSockets_;
    std::string             configFile_;
    int                     epollfd_;

    auto setupListenSocket(std::string ip, int port) -> std::expected<int, std::string>;
    auto setupListenSockets() -> std::expected<void, std::string>;

    
    auto getListenServerAddress(std::string ip, int port) -> std::expected<sockaddr_in, std::string>;

    auto createConnection(int listenSocket) -> std::expected<void, std::string>;
    auto handleEvent(int fd) -> std::expected<int, std::string>;
    auto setNonBlocking(int connSock) -> std::expected<void, std::string>;

    // cleanup
    auto closeListenSockets() -> void;

};

#endif // SERVER_HPP