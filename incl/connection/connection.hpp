#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "parsing.hpp"
#define MAX_EVENTS 10
#define BUFFER_SIZE 100000

enum class ClientState
{
    Receiving,
    Processing,
    Sending,
    Sent,
    Closed,
    Error,
};

enum class ErrorType
{
    None,
    Timeout,
    ConnectionReset,
    Unknown,
};

struct Client
{
        int                           socketfd;
        std::tuple<std::string, int>& listenSocketIpPortPair;
        std::string                   service;
        std::string                   host;

        ClientState  state;
        HTTPRequest  request;
        HTTPResponse response;
        ErrorType    error;
};

auto setNonBlocking(int socketfd) -> std::expected<void, std::string>;

#endif // CONNECTION_HPP
