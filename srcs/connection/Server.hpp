#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "connection.hpp"

class Server
{
public:
    Server();
    ~Server();

    void        connection_loop();


private:
    struct epoll_event  ev_;
    struct epoll_event  events_[MAX_EVENTS];
    int                 epollfd_;

    int         setupListenSocket(int port, std::string ip);
    sockaddr_in setListenServerAddress(int port, std::string ip);

    void        createConnection(int listenSocket);
    int         handleEvent(int fd);
    void        setNonBlocking(int connSock);

    
};

#endif // SERVER_HPP