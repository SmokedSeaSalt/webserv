#ifndef CONFIGPARSING_HPP
#define CONFIGPARSING_HPP

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <expected>

struct AcceptedMethods
{
    bool getAllowed = false;
    bool headAllowed = false;
    bool postAllowed = false;
    bool deleteAllowed = false;
};

struct Location
{
    // The end of line comments correspond to the name of the directive in the conf file
    std::string                         pathPrefix; // location /pathPrefix
    AcceptedMethods                     acceptedMethods; // methods
    int                                 redirectCode = 0; // return
    std::string                         redirectLocation; // return
    std::string                         root; // root
    bool                                directoryListing = false; // autoindex
    std::string                         defaultFile; // index
    bool                                uploadsAllowed = false; // upload_store
    std::string                         uploadLocation; // upload_store
    std::map<std::string, std::string>  cgiPaths; // cgi <cgi type, cgi path>
};

struct ServerBlock
{
    std::string                 ip;
    int                         port;
    std::map<int, std::string>  defaultErrorPages; // <error code, path to default error page>
    std::size_t                 maxBodySize;
    std::vector<Location>       locations;
};

struct Config
{
    std::vector<ServerBlock>   serverBlocks; // interface:port pairs
};

auto	parseConfigFile(std::string configFile) -> std::expected<Config, std::string>;
auto    parseServerBlock(std::ifstream& inFile) -> std::expected<ServerBlock, std::string>;
auto    parseLocation(std::ifstream& inFile, std::string pathPrefix) -> std::expected<Location, std::string>;
auto    split(std::string line) -> std::expected<std::vector<std::string>, std::string>;
auto    trimTrailingSemicolon(std::string& str) -> std::expected<void, std::string>;
#endif // CONFIGPARSING_HPP
