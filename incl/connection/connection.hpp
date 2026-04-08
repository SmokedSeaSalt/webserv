#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "parsing.hpp"
#include "HTTPRequest.hpp"
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
        int         socketfd;
        ClientState state;
        HTTPRequest request; // todo: change this to HTTPRequest when merge with mathijs
        HTTPMessage response;
        ErrorType   error;
};

#endif // CONNECTION_HPP
