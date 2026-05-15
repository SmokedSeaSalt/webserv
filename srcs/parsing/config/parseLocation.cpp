#include "configParsing.hpp"
#include "parsing.hpp"
#include <expected>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace Config
{

static auto parseMethods(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    AcceptedMethods methods;

    if (tokens.size() < 2)
        return std::unexpected("Invalid method argument count");
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
        return std::unexpected("Invalid redirect argument count");
    try
    {
        size_t pos            = 0;
        location.redirectCode = std::stoi(tokens[1], &pos);
        if (pos != tokens[1].size())
            return std::unexpected("Invalid redirect code");
    }
    catch (...)
    {
        return std::unexpected("Invalid redirect code");
    }
    location.redirectLocation = tokens[2];
    return {};
}

static auto parseRoot(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 2)
        return std::unexpected("Invalid root argument count");
    location.root = tokens[1];
    return {};
}

static auto parseAutoIndex(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 2)
        return std::unexpected("Invalid autoindex argument count");
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
        return std::unexpected("Invalid index argument count");
    location.defaultFile = tokens[1];
    return {};
}

static auto parseUploadStore(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 2)
        return std::unexpected("Invalid upload store argument count");
    location.uploadLocation = tokens[1];
    location.uploadsAllowed = true;
    return {};
}

static auto isValidExecutable(std::string path)
{
    if (!std::filesystem::exists(path) || std::filesystem::is_directory(path))
        return false;

    if (access(path.c_str(), X_OK) == 0)
        return true;

    return false;
}

static auto parseCGI(Location& location, std::vector<std::string> tokens)
    -> std::expected<void, std::string>
{
    if (tokens.size() != 3)
        return std::unexpected("Invalid cgi argument count");
    if (location.cgiPaths.count(tokens[1]))
        return std::unexpected("Duplicate CGI extension not allowed");
    if (!isValidExecutable(tokens[2]))
        return std::unexpected("CGI binary path not executable or existent");
    location.cgiPaths[tokens[1]] = tokens[2];
    return {};
}

auto isDirectiveAlone(std::string directive, const std::map<std::string, bool>& visited) -> std::expected<void, std::string>
{
    if (visited.at(directive))
        for (const auto& curDirective : visited)
            if (curDirective.first != directive && curDirective.second)
                return std::unexpected("No other directives allowed with " + directive + " in location block");
    return {};
}

auto isUploadStoreLocationValid(Location location, const std::map<std::string, bool>& visited) -> std::expected<void, std::string>
{
    if (!visited.at("upload_store"))
    {
        if (location.acceptedMethods.postAllowed)
            return std::unexpected("Method POST only allowed if upload_store present");
        if (location.acceptedMethods.deleteAllowed)
            return std::unexpected("Method DELETE only allowed if upload_store present");
        return {};
    }

    if (!visited.at("methods"))
        return std::unexpected("Locations with upload_store should have a methods directive");
    
    if (!(location.acceptedMethods.postAllowed || location.acceptedMethods.deleteAllowed) || location.acceptedMethods.getAllowed || location.acceptedMethods.headAllowed)
        return std::unexpected("Locations with upload_store should only contain POST and/or DELETE and must not contain any other methods");
    
    for (const auto& curDirective : visited)
    {
        if ((curDirective.first == "upload_store" && curDirective.second) || (curDirective.first == "methods" && curDirective.second))
            continue;
        if (curDirective.second)
            return std::unexpected("No other directives allowed with upload_store and methods in location block");
    }
    return {};
}

auto parseLocation(std::ifstream& inFile, std::string pathPrefix)
    -> std::expected<Location, std::string>
{
    Location                                                                                                              location;
    std::string                                                                                                           buf;
    std::vector<std::string>                                                                                              tokens;
    std::map<std::string, std::function<std::expected<void, std::string>(Location & location, std::vector<std::string>)>> functionMap{
        {"methods", parseMethods},
        {"return", parseRedirect},
        {"root", parseRoot},
        {"autoindex", parseAutoIndex},
        {"index", parseIndex},
        {"upload_store", parseUploadStore},
        {"cgi", parseCGI},
    };
    std::map<std::string, bool> visited{
        {"methods", false},
        {"return", false},
        {"root", false},
        {"autoindex", false},
        {"index", false},
        {"upload_store", false},
    };

    location.pathPrefix = pathPrefix;
    while (std::getline(inFile, buf))
    {
        buf = stringTrim(buf);
        if (buf == "}")
        {
            auto res = isDirectiveAlone("return", visited);
            if (!res.has_value())
                return std::unexpected(res.error());
            res = isUploadStoreLocationValid(location, visited);
            if (!res.has_value())
                return std::unexpected(res.error());
            if (location.uploadAllowed)
            {
                std::string tmpUploadLocation;
                if (location.uploadLocation.length() > 0 && location.uploadLocation[0] == '/')
                    tmpUploadLocation = location.uploadLocation.substr(1);
                location.absoluteUploadStorePath = std::filesystem::path(InputArgs::args.relativePath) / tmpUploadLocation;
            }
            return location;
        }
        if (buf.empty())
            continue;

        auto semicolonResult = trimTrailingSemicolon(buf);
        if (!semicolonResult.has_value())
            return std::unexpected(semicolonResult.error());

        auto splitResult = split(buf);
        if (!splitResult.has_value())
            return std::unexpected(splitResult.error() + " at: " + buf);
        tokens = splitResult.value();
        if (tokens.size() < 2)
            return std::unexpected("Invalid location directive argument count");

        if (functionMap.count(tokens[0]) != 1)
            return std::unexpected("Directive not found: " + tokens[0]);

        if (visited.count(tokens[0]))
        {
            if (visited[tokens[0]] == true)
                return std::unexpected("Duplicate " + tokens[0] + " found");
            visited[tokens[0]] = true;
        }
        auto result = functionMap[tokens[0]](location, tokens);
        if (!result.has_value())
            return std::unexpected(result.error());
    }
    return std::unexpected("Location block closing bracket not found");
}

} // namespace Config
