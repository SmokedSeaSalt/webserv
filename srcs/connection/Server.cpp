#include "connection.hpp"
#include <map>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Server.hpp"
#include <fcntl.h>

Server::Server()
{

}
    
Server::~Server()
{
    
}

void Server::setNonBlocking(int socketfd)
{
    if (fcntl(socketfd, F_SETFL, O_NONBLOCK) == -1)
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

#include <iostream>
#include <unistd.h>

void Server::handleEvent(int fd)
{
    char buf[10000];
    read(fd, buf, 10000);
    std::cout << buf << std::endl;
}

void Server::createConnection(int listenSocket)
{
    int connectionSocket;

    connectionSocket = accept(listenSocket, nullptr, nullptr); // todo: can provide more args to log info on clients
    if (connectionSocket == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    setNonBlocking(connectionSocket);
    ev_.data.fd = connectionSocket;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, connectionSocket, &ev_) == -1)
    {
        perror("epoll_ctl: connectionSocket");
        exit(EXIT_FAILURE);
    }
}

sockaddr_in Server::setListenServerAddress(int port, std::string ip)
{
    sockaddr_in listenServerAddress;

    listenServerAddress.sin_family = AF_INET;
    listenServerAddress.sin_port = htons(port);
    if (ip.empty())
        listenServerAddress.sin_addr.s_addr = INADDR_ANY;
    else if (inet_pton(AF_INET, ip.c_str(), &listenServerAddress.sin_addr) != 1) // todo: check two different fail cases?
    {
        perror("inet_pton");
        exit(1);
    }
    return listenServerAddress;
}

int Server::setupListenSocket(int port, std::string ip)
{
    // Create socket with IPv4 and TCP
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE); // TODO: exit or return?
    }

    sockaddr_in listenServerAddress = setListenServerAddress(port, ip);
    if (bind(listenSocket, (struct sockaddr*)&listenServerAddress, sizeof(listenSocket)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listenSocket, SOMAXCONN) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    setNonBlocking(listenSocket);
    return listenSocket;
}

void Server::connection_loop()
{
    std::map<int, Client> clientMap;
    
    int                   listenSocket, nfds;

    listenSocket = setupListenSocket(8080, "");


    epollfd_ = epoll_create(1);
    if (epollfd_ == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev_.events  = EPOLLIN;
    ev_.data.fd = listenSocket;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, listenSocket, &ev_) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        nfds = epoll_wait(epollfd_, events_, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        

        for (int n = 0; n < nfds; ++n)
        {
            if (events_[n].data.fd == listenSock)
                createConnection(events_[n].data.fd);
            else
                handleEvent(events_[n].data.fd);
        }
    }
}
