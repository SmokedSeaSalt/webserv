#include "configParsing.hpp"
#include "connection.hpp"
#include "logging.hpp"
#include "parsing.hpp"
#include <expected>
#include <fstream>
#include <string>

namespace Config
{

static auto isDuplicateLocationPathPrefix(const ServerBlock& serverBlock, const Location& location) -> bool
{
    for (const Location& existing : serverBlock.locations)
    {
        if (location.pathPrefix == existing.pathPrefix)
            return true;
    }
    return false;
}

static auto isDuplicateErrorPage(const ServerBlock& serverBlock, const int& errorCode) -> bool
{
    for (const auto& [existingErrorCode, existingErrorPage] : serverBlock.defaultErrorPages)
    {
        if (errorCode == existingErrorCode)
            return true;
    }
    return false;
}

static auto parseErrorPage(ServerBlock& serverBlock, std::string buf)
    -> std::expected<void, std::string>
{
    std::vector<std::string> tokens;
    int                      errorCode;

    auto semicolonResult = trimTrailingSemicolon(buf);
    if (!semicolonResult.has_value())
        return std::unexpected(semicolonResult.error());

    auto splitResult = split(buf);
    if (!splitResult.has_value())
        return std::unexpected(splitResult.error());
    tokens = splitResult.value();
    if (tokens.size() != 3)
        return std::unexpected("Invalid argument count at: " + buf);
    try
    {
        size_t pos = 0;
        errorCode  = std::stoi(tokens[1], &pos);
        if (pos != tokens[1].size())
            return std::unexpected("Invalid error code at: " + buf);
        // todo: what are valid int values for error codes?
    }
    catch (...)
    {
        return std::unexpected("Invalid error code at: " + buf);
    }
    if (tokens[2].empty())
        return std::unexpected("Invalid error page at: " + buf);
    if (!isDuplicateErrorPage(serverBlock, errorCode))
        serverBlock.defaultErrorPages[errorCode] = tokens[2];
    else
        return std::unexpected(("Duplicate error page found for: " + std::to_string(errorCode)));
    return {};
}

static auto parseMaxBodySize(ServerBlock& serverBlock, std::string buf)
    -> std::expected<void, std::string>
{
    std::vector<std::string> tokens;
    int                      maxBodySize;

    auto semicolonResult = trimTrailingSemicolon(buf);
    if (!semicolonResult.has_value())
        return std::unexpected(semicolonResult.error());

    auto splitResult = split(buf);
    if (!splitResult.has_value())
        return std::unexpected(splitResult.error());
    tokens = splitResult.value();
    if (tokens.size() != 2)
        return std::unexpected("Invalid client_max_body_size argument count at: " + buf);
    try
    {
        size_t pos  = 0;
        maxBodySize = std::stoi(tokens[1], &pos);
        if (pos != tokens[1].size())
            return std::unexpected("Invalid max body size at: " + buf);
    }
    catch (...)
    {
        return std::unexpected("Invalid error code at: " + buf);
    }
    if (maxBodySize < 0)
        return std::unexpected("Invalid error code at: " + buf);
    serverBlock.maxBodySize = maxBodySize;

    return {};
}

static auto parseListen(ServerBlock& serverBlock, std::string buf)
    -> std::expected<void, std::string>
{
    std::vector<std::string> tokens;
    std::string              ip;
    std::string              portStr;
    int                      port;

    auto semicolonResult = trimTrailingSemicolon(buf);
    if (!semicolonResult.has_value())
        return std::unexpected(semicolonResult.error());

    auto splitResult = split(buf);
    if (!splitResult.has_value())
        return std::unexpected(splitResult.error());
    tokens = splitResult.value();
    if (tokens.size() != 2)
        return std::unexpected("Invalid argument count at: " + buf);

    std::size_t colonIndex = tokens[1].rfind(':');
    if (colonIndex == std::string::npos)
        return std::unexpected("Invalid listen format at: " + buf);

    ip      = tokens[1].substr(0, colonIndex);
    portStr = tokens[1].substr(colonIndex + 1);
    if (ip.empty() || portStr.empty())
        return std::unexpected("Invalid listen format at: " + buf);

    try
    {
        std::size_t pos = 0;
        port            = std::stoi(portStr, &pos);
        if (pos != portStr.size())
            return std::unexpected("Invalid port at: " + buf);
    }
    catch (...)
    {
        return std::unexpected("Invalid port at: " + buf);
    }

    if (port < 1 || port > 65535)
        return std::unexpected("Invalid port at: " + buf);

    serverBlock.ip   = ip;
    serverBlock.port = port;
    return {};
}

auto parseServerBlock(std::ifstream& inFile) -> std::expected<ServerBlock, std::string>
{
    ServerBlock serverBlock;
    std::string buf;
    bool foundListen = false, foundMaxBodySize = false;

    while (std::getline(inFile, buf))
    {
        buf = stringTrim(buf);
        if (buf.find("listen") == 0)
        {
            if (foundListen)
                return std::unexpected("duplicate listen found");
            auto result = parseListen(serverBlock, buf);
            if (!result.has_value())
                return std::unexpected(result.error());
            foundListen = true;
        }
        else if (buf.find("client_max_body_size") == 0)
        {
            if (foundMaxBodySize)
                return std::unexpected("duplicate client_max_body_size found");
            auto result = parseMaxBodySize(serverBlock, buf);
            if (!result.has_value())
                return std::unexpected(result.error());
            foundMaxBodySize = true;
        }
        else if (buf.find("error_page") == 0)
        {
            auto result = parseErrorPage(serverBlock, buf);
            if (!result.has_value())
                return std::unexpected(result.error());
        }
        else if (buf.find("location") == 0)
        {
            auto splitResult = split(buf);
            if (!splitResult.has_value())
                return std::unexpected(splitResult.error());
            if (splitResult.value().size() != 3 ||
                !(splitResult.value()[0] == "location" && splitResult.value()[2] == "{"))
                return std::unexpected("Parse error at: " + buf);
            auto result = parseLocation(inFile, splitResult.value()[1]);
            if (!result.has_value())
                return std::unexpected(result.error());
            if (!isDuplicateLocationPathPrefix(serverBlock, result.value()))
                serverBlock.locations.push_back(result.value());
            else
                return std::unexpected("Duplicate location path prefix");
        }
        else if (buf == "}")
            return serverBlock;
    }
    return std::unexpected("Server block closing bracket not found");
}

} // namespace Config