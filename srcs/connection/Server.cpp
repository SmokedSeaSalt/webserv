#include "connection.hpp"
#include <map>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Server.hpp"

void Server::setNonBlocking(int connSock)
{
	
}

void Server::handleEvent(int fd)
{

}

void Server::createConnection()
{
    int conn_sock;

    conn_sock = accept(listen_sock, (struct sockaddr*)&addr, &addrlen);
    if (conn_sock == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    setNonBlocking(conn_sock);
    ev.data.fd = conn_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
    {
        perror("epoll_ctl: conn_sock");
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

int Server::setupListenSock(int port, std::string ip)
{
    // Create socket with IPv4 and TCP
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == -1)
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

}

void Server::connection_loop()
{
    std::map<int, Client> clientMap;
    struct epoll_event    ev, events[MAX_EVENTS];
    int                   listenSock, nfds, epollfd;

    /* Code to set up listening socket, 'listen_sock',
    (socket(), bind(), listen()) omitted.  */
    listenSock = setupListenSock();

    epollfd = epoll_create(1);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events  = EPOLLIN;
    ev.data.fd = listenSock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenSock, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
            perror("epoll_wait"); exit(EXIT_FAILURE);
        

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == listenSock)
                createConnection();
            else
                handleEvent(events[n].data.fd);
        }
    }
}
