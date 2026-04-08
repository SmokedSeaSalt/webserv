#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "parsing.hpp"
#define MAX_EVENTS 10
#define BUFFER_SIZE 10

enum class ClientState
{
    Receiving,
    Processing,
    Sending,
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
        int          socketfd;
        ClientState  state;
        HTTPRequest  request; // todo: change this to HTTPRequest when merge with mathijs
        HTTPResponse response;
        ErrorType    error;
};

#endif // CONNECTION_HPP
