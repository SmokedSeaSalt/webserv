#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parsing.hpp"
#define MAX_EVENTS 10


enum class SocketState
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
        int         socketfd;
        SocketState state;
        HTTPMessage request;
        HTTPMessage response;
        ErrorType   error;
};

#endif // CONNECTION_HPP
