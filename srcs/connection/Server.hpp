#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <netinet/in.h>

class Server
{
public:
    Server();
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();

private:
    int listenSock;

    void        connection_loop();
    int         setupListenSock(int port, std::string ip);
    sockaddr_in setListenServerAddress(int port, std::string ip);

    void        createConnection();
    void        handleEvent(int fd);
    void        setNonBlocking(int connSock);

    
};

#endif // SERVER_HPP