#include "configParsing.hpp"
#include <string>
#include <expected>
#include <fstream>
#include "parsing.hpp"

static auto	parseErrorPage(ServerBlock& serverBlock, std::string buf) -> std::expected<void, std::string>
{
    auto                        splitResult = split(buf);
    std::vector<std::string>    words;
    int                         errorCode;
    int                         semicolonIndex;
    std::string                 errorPage;

    if (!splitResult.has_value())
        return std::unexpected(splitResult.error());
    words = splitResult.value();
    if (words.size() != 3)
        return std::unexpected("Invalid argument count at: " + buf);
    try {
        errorCode = std::stoi(words[1]);
        // todo: what are valid int values for error codes?
    } catch (...) {
        return std::unexpected("Invalid error code at: " + buf);
    }
    semicolonIndex = words[2].find(';');
    if (semicolonIndex != words.size() - 1)
        return std::unexpected("Semicolon error at: " + buf);
    serverBlock.defaultErrorPages[errorCode] = words[2].substr(0, words[2].length() - 1);
    return {};
}

static auto	parseMaxBodySize(ServerBlock& serverBlock, std::string buf) -> std::expected<void, std::string>
{


    return {};
}

static auto	parseListen(ServerBlock& serverBlock, std::string buf) -> std::expected<void, std::string>
{


    return {};
}

auto	parseServerBlock(std::ifstream& inFile) -> std::expected<ServerBlock, std::string>
{
    ServerBlock serverBlock;
    std::string buf;

    while (std::getline(inFile, buf))
    {
        buf = stringTrim(buf);
        if (buf.find("listen") == 0)
        {
            auto tmp = parseListen(serverBlock, buf);
            if (!tmp.has_value())
                return std::unexpected(tmp.error());
        }
        else if (buf.find("client_max_body_size") == 0)
        {
            auto tmp = parseMaxBodySize(serverBlock, buf);
            if (!tmp.has_value())
                return std::unexpected(tmp.error());
        }
        else if (buf.find("error_page") == 0)
        {
            auto tmp = parseErrorPage(serverBlock, buf);
            if (!tmp.has_value())
                return std::unexpected(tmp.error());
        }
        else if (buf.find("location") == 0)
        {
            auto tmp = parseLocation(inFile);
            if (!tmp.has_value())
                return std::unexpected(tmp.error());
            serverBlock.locations.push_back(tmp.value());
        }
    }
}