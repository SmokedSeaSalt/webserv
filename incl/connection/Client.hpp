#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "connection.hpp"
#include <sys/epoll.h>

enum class HandleEventResult;

enum class ClientState
{
    Receiving,
    Processing,
    Sending,
    Sent,
    Closed,
    Error,
};

class Client
{
    public:
        Client(int socketfd, int listendSocketPort, std::string service, std::string host);
        auto handleEvent(const epoll_event& epollEvent) -> HandleEventResult;

        // get/set
        auto setSocketfd(int) -> void;
        auto getSocketfd() -> int;

        auto setListenSocketPort(int) -> void;
        auto getListenSocketPort() -> int;

        auto setService(std::string) -> void;
        auto getService() -> std::string;

        auto setHost(std::string) -> void;
        auto getHost() -> std::string;

        auto setState(ClientState) -> void;
        auto getState() -> ClientState;

    private:
        int         socketfd_;
        int         listenSocketPort_;
        std::string service_;
        std::string host_;

        ClientState  state_;
        HTTPRequest  request_;
        HTTPResponse response_;
        ErrorType    error_;
};

#endif // CLIENT_HPP