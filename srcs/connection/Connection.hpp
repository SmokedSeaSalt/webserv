#ifndef CONNECTION_HPP
#define CONNECTION_HPP

class Connection
{
public:
    Connection();
    Connection(const Connection& other);
    Connection& operator=(const Connection& other);
    ~Connection();

private:
    int listenSock;
    
};

#endif // CONNECTION_HPP