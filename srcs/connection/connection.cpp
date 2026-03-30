// #include "connection.hpp"
// #include <map>
// #include <sys/epoll.h>
// #include <sys/socket.h>

// void setNonBlocking()
// {

// }

// void handleEvent(int fd)
// {

// }

// void createConnection()
// {
//     int conn_sock;

//     conn_sock = accept(listen_sock, (struct sockaddr*)&addr, &addrlen);
//     if (conn_sock == -1)
//     {
//         perror("accept");
//         exit(EXIT_FAILURE);
//     }
//     setNonBlocking(conn_sock);
//     ev.data.fd = conn_sock;
//     if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
//     {
//         perror("epoll_ctl: conn_sock");
//         exit(EXIT_FAILURE);
//     }
// }

// int setupListenSock()
// {
//     /* Code to set up listening socket, 'listen_sock',
//     (socket(), bind(), listen()) omitted.  */
//     return 10;
// }

// void connection_loop()
// {
//     std::map<int, Client> clientMap;
//     struct epoll_event    ev, events[MAX_EVENTS];
//     int                   listenSock, nfds, epollfd;

//     /* Code to set up listening socket, 'listen_sock',
//     (socket(), bind(), listen()) omitted.  */
//     listenSock = setupListenSock();

//     epollfd = epoll_create(1);
//     if (epollfd == -1)
//     {
//         perror("epoll_create1");
//         exit(EXIT_FAILURE);
//     }

//     ev.events  = EPOLLIN;
//     ev.data.fd = listenSock;
//     if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenSock, &ev) == -1)
//     {
//         perror("epoll_ctl: listen_sock");
//         exit(EXIT_FAILURE);
//     }

//     while (true)
//     {
//         nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
//         if (nfds == -1)
//             perror("epoll_wait"); exit(EXIT_FAILURE);
        

//         for (int n = 0; n < nfds; ++n)
//         {
//             if (events[n].data.fd == listenSock)
//                 createConnection();
//             else
//                 handleEvent(events[n].data.fd);
//         }
//     }
// }
