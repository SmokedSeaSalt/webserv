#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Cgi.hpp"
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

class Client : public std::enable_shared_from_this<Client>

{
    public:
        Client(int socketfd, int listendSocketPort, std::tuple<std::string, int> listenSocketIpPortPair, std::string service, std::string host);
        auto handleEvent(const epoll_event& epollEvent) -> HandleEventResult;

        // get/set
        auto setSocketfd(int) -> void;
        auto getSocketfd() -> int;

        auto setListenSocketPort(int) -> void;
        auto getListenSocketPort() -> int;

        auto setCgiPID(int) -> void;
        auto getCgiPID() -> int;

        auto setService(std::string) -> void;
        auto getService() -> std::string;

        auto setHost(std::string) -> void;
        auto getHost() -> std::string;

        auto setState(ClientState) -> void;
        auto getState() -> ClientState&;

        auto setRequest(HTTPRequest) -> void;
        auto getRequest() -> HTTPRequest&;

        auto setResponse(HTTPResponse) -> void;
        auto getResponse() -> HTTPResponse&;

        auto setListenSocketIpPortPair(std::tuple<std::string, int>) -> void;
        auto getListenSocketIpPortPair() -> std::tuple<std::string, int>;

        auto updateLastActivityTime() -> void;
        std::chrono::steady_clock::time_point getLastActivity();

    private:
        int                                   socketfd_;
        int                                   cgifd_;
        int                                   listenSocketPort_;
        int                                   cgiPID_ = -1;
        std::tuple<std::string, int>          listenSocketIpPortPair_;
        std::string                           service_;
        std::string                           host_;
        bool                                  requestIsCgi_;
        std::chrono::steady_clock::time_point lastActivityTime_;

        ClientState  state_;
        HTTPRequest  request_;
        HTTPResponse response_;
        Cgi          CgiHandler_;
        ErrorType    error_;
};

#endif // CLIENT_HPP