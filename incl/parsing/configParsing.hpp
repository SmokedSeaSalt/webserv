#include <vector>
#include <string>
#include <map>

struct ServerBlock
{
        std::string ip;
        int         port;
        // other rules per serverblock
};

struct Config
{
        std::map<int, std::string> defaultErrorPages; // <error code, default error message>
        std::size_t                maxBodySize;
        std::vector<ServerBlock>   serverBlocks;
};