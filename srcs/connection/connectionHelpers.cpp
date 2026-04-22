#include <expected>
#include <fcntl.h>
#include <string>

auto setNonBlocking(int socketfd) -> std::expected<void, std::string>
{
    if (fcntl(socketfd, F_SETFL, O_NONBLOCK) == -1)
    {
        perror("fcntl");
        return std::unexpected("fcntl failed");
    }
    return {};
}