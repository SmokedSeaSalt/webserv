#include "configParsing.hpp"
#include "parsing.hpp"
#include <expected>
#include <fstream>
#include <functional>
#include <string>
 
static auto parseMethods(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    AcceptedMethods methods;
    for (size_t i = 1; i < tokens.size(); i++)
    {
        if (tokens[i] == "GET")
        {
            if (methods.getAllowed)
                return std::unexpected("Duplicate method not allowed");
            methods.getAllowed = true;
        }
        else if (tokens[i] == "HEAD")
        {
            if (methods.headAllowed)
                return std::unexpected("Duplicate method not allowed");
            methods.headAllowed = true;
        }

        else if (tokens[i] == "POST")
        {
            if (methods.postAllowed)
                return std::unexpected("Duplicate method not allowed");
            methods.postAllowed = true;
        }
        else if (tokens[i] == "DELETE")
        {
            if (methods.deleteAllowed)
                return std::unexpected("Duplicate method not allowed");
            methods.deleteAllowed = true;
        }
        else
            return std::unexpected("Invalid method: " + tokens[i]);
    }
    location.acceptedMethods = methods;
    return {};
}

static auto parseRedirect(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 3)
            return std::unexpected("Invalid argument count");
    try {
        location.redirectCode = std::stoi(tokens[1]);
    } catch (...) {
        return std::unexpected("Invalid redirect code");
    }
    location.redirectLocation = tokens[2];
    return {};
}

static auto parseRoot(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 2)
            return std::unexpected("Invalid argument count");
    location.root = tokens[1];
    return {};
}

static auto parseAutoIndex(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 2)
            return std::unexpected("Invalid argument count");
    if (tokens[1] == "on")
        location.directoryListing = true;
    else if (tokens[1] == "off")
        location.directoryListing = false;
    else
        return std::unexpected("Invalid autoindex mode");
    return {};
}

static auto parseIndex(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 2)
            return std::unexpected("Invalid argument count");
    location.defaultFile = tokens[1];
    return {};
}

static auto parseUploadStore(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 2)
        return std::unexpected("Invalid argument count");
    location.uploadLocation = tokens[1];
    location.uploadsAllowed = true;
    return {};
}

static auto parseCGI(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 3)
        return std::unexpected("Invalid argument count");
    location.cgiPaths[tokens[1]] = tokens[2];
    return {};
}

auto parseLocation(std::ifstream& inFile, std::string pathPrefix) -> std::expected<Location, std::string>
{
    Location    location;
    std::string buf;
    std::vector<std::string> tokens;
    std::map<std::string, std::function<std::expected<void, std::string>(Location& location, std::vector<std::string>)>>
        functionMap{
            {"methods", parseMethods}, {"return", parseRedirect},
            {"root", parseRoot},       {"autoindex", parseAutoIndex},
            {"index", parseIndex},     {"upload_store", parseUploadStore},
            {"cgi", parseCGI},
        };

    location.pathPrefix = pathPrefix;
    while (std::getline(inFile, buf))
    {
        buf = stringTrim(buf);
        trimTrailingSemicolon(buf);
        if (buf == "}")
            return location;
        auto splitResult = split(buf);
        if (!splitResult.has_value())
            return std::unexpected(splitResult.error());
        tokens = splitResult.value();
        if (tokens.size() < 2)
            return std::unexpected("Invalid argument count");

        if (functionMap.count(tokens[0]) != 1)
            return std::unexpected("Directive not found: " + tokens[0]);
        auto result = functionMap[tokens[0]](location, tokens);
        if (!result.has_value())
            return std::unexpected(result.error());
    }
    return std::unexpected("Location block closing bracket not found");
}







 
        // if (buf.find("methods") == 0)
        // {
        //     auto result = parseMethods(location, buf);
        //     if (!result.has_value())
        //         return std::unexpected(result.error());
        // }
        // else if (buf.find("return") == 0)
        // {
        //     auto result = parseRedirect(location, buf);
        //     if (!result.has_value())
        //         return std::unexpected(result.error());
        // }
        // else if (buf.find("root") == 0)
        // {
        //     auto result = parseRoot(location, buf);
        //     if (!result.has_value())
        //         return std::unexpected(result.error());
        // }
        // else if (buf.find("autoindex") == 0)
        // {
        //     auto result = parseAutoIndex(location, buf);
        //     if (!result.has_value())
        //         return std::unexpected(result.error());
        // }
        // else if (buf.find("index") == 0)
        // {
        //     auto result = parseIndex(location, buf);
        //     if (!result.has_value())
        //         return std::unexpected(result.error());
        // }
        // else if (buf.find("upload_store") == 0)
        // {
        //     auto result = parseUploadStore(location, buf);
        //     if (!result.has_value())
        //         return std::unexpected(result.error());
        // }
        // else if (buf.find("cgi") == 0)
        // {
        //     auto result = parseCGI(location, buf);
        //     if (!result.has_value())
        //         return std::unexpected(result.error());
        // }
