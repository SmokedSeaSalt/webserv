// #include "connection.hpp"
#include "configParsing.hpp"
#include "logging.hpp"
#include <expected>

namespace Config
{

auto getServerBlock(Config& config, std::tuple<std::string, int>& listenSocketIpPortPair) -> std::expected<ServerBlock, std::string>
{
    for (ServerBlock& serverBlock : config.serverBlocks)
    {
        if (serverBlock.ip == get<0>(listenSocketIpPortPair) && serverBlock.port == get<1>(listenSocketIpPortPair))
            return serverBlock;
    }
    return std::unexpected("Serverblock not found");
}

auto getLocation(ServerBlock& serverBlock, std::string target) -> std::expected<Location, std::string>
{
    // std::string target          = client.request.getMessage().requestTarget;
    int      maxMatchedLen   = 0;
    int      curPathLen      = 0;
    Location matchedLocation = {};
    bool     foundMatch      = false;

    for (Location curLocation : serverBlock.locations)
    {
        curPathLen = curLocation.pathPrefix.length();
        if (target.find(curLocation.pathPrefix) != 0)
            continue;
        if (curPathLen > maxMatchedLen)
        {
            maxMatchedLen   = curPathLen;
            matchedLocation = curLocation;
            foundMatch      = true;
        }
    }
    if (foundMatch)
        return matchedLocation;
    LOG(LogLevel::kInfo, "Client with target: {}. No matching location found.\n", target);
    return std::unexpected("Location not found");
}

// auto getServerBlock(Config& config, Client& client) -> std::expected<ServerBlock, std::string>
// {
//     for (ServerBlock& serverBlock : config.serverBlocks)
//     {
//         if (serverBlock.ip == get<0>(client.listenSocketIpPortPair) && serverBlock.port == get<1>(client.listenSocketIpPortPair))
//             return serverBlock;
//     }
//     return std::unexpected("Serverblock not found");
// }

// auto getLocation(ServerBlock& serverBlock, Client& client) -> std::expected<Location, std::string>
// {
//     std::string target          = client.request.getMessage().requestTarget;
//     int         maxMatchedLen   = 0;
//     int         curPathLen      = 0;
//     Location    matchedLocation = {};
//     bool        foundMatch      = false;

//     for (Location curLocation : serverBlock.locations)
//     {
//         curPathLen = curLocation.pathPrefix.length();
//         if (target.find(curLocation.pathPrefix) != 0)
//             continue;
//         if (curPathLen > maxMatchedLen)
//         {
//             maxMatchedLen   = curPathLen;
//             matchedLocation = curLocation;
//             foundMatch      = true;
//         }
//     }
//     if (foundMatch)
//         return matchedLocation;
//     LOG(LogLevel::kInfo, "Client with fd:{} with target: {}. No matching location found.\n", client.socketfd, target);
//     return std::unexpected("Location not found");
// }
// targets [/, /images, /images/pngs] for target /images/pngs/cat.png

} // namespace Config