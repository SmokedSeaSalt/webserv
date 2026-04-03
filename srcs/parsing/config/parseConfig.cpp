#include "configParsing.hpp"
#include <string>
#include <expected>
#include <fstream>
#include "parsing.hpp"

auto	parseConfigFile(std::string configFile) -> std::expected<Config, std::string>
{
    Config          config;
    std::ifstream   inFile;
    std::string     buf;

    inFile.open(configFile);
    if (!inFile.is_open())
        return std::unexpected("Config file could not be opened");

    while (std::getline(inFile, buf))
    {
        buf = stringTrim(buf);
        
        if (buf == "server {")
        {
            auto serverBlock = parseServerBlock(inFile);
            if (!serverBlock.has_value())
                return std::unexpected(serverBlock.error());
            config.serverBlocks.push_back(serverBlock.value());
        }
        else if (!buf.empty())
        {
            return std::unexpected("Error at: " + buf);
        }
    }
    return config;
}