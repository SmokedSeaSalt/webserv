#include "parsing.hpp"

struct HTTPMessage;

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