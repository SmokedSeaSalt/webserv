#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <expected>
#include <string>

#define MAX_EVENTS 10
#define BUFFER_SIZE 100000

enum class ErrorType
{
    None,
    Timeout,
    ConnectionReset,
    Unknown,
};

auto setNonBlocking(int socketfd) -> std::expected<void, std::string>;

#endif // CONNECTION_HPP
