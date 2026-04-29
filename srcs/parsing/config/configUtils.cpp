// #include "connection.hpp"
#include "configParsing.hpp"
#include "HTTPRequest.hpp"
#include "logging.hpp"
#include <expected>

namespace Config
{

auto getServerBlock(Config& config, const std::tuple<std::string, int>& listenSocketIpPortPair) -> std::expected<ServerBlock, std::string>
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

auto checkContentLength(HTTPRequest request) -> ResponseStatusCode
{
    size_t contentLength = 0;
    try
    {
        if (request.getMessage().headers.contains("content-length"))
        {
            int tmp = std::stoi(request.getMessage().headers["content-length"][0]);
            if (tmp < 0)
                return ResponseStatusCode::kBadRequest;
            contentLength = static_cast<size_t>(tmp);
        }
    }
    catch (std::exception& e)
    {
        LOG(LogLevel::kDebug, "content length stoi failed with e: {}", e.what());
        return ResponseStatusCode::kBadRequest;
    }
    const ServerBlock& serverBlock = request.getServerBlock();
    if (contentLength > serverBlock.maxBodySize)
    {
        return ResponseStatusCode::kContentTooLarge;
    }
    return ResponseStatusCode::kOK;
}

auto checkLocationCompliance(HTTPRequest request) -> ResponseStatusCode
{
    Location location = request.getLocation();
    if (!location.acceptedMethods.isAllowed(request.getMessage().method))
    {
        LOG(LogLevel::kDebug, "Method {} not allowed", request.getMessage().method);
        return ResponseStatusCode::kMethodNotAllowed;
    }
    // todo are there other checks?
    return ResponseStatusCode::kOK;
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