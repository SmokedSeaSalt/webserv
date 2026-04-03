#ifndef CONFIGPARSING_HPP
#define CONFIGPARSING_HPP

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <expected>

struct AcceptedMethods
{
    bool    getAllowed;
    bool    headAllowed;
    bool    postAllowed;
    bool    deleteAllowed;
};

struct Location
{
    AcceptedMethods                     acceptedMethods;
    std::string                         redirectLocation;
    int                                 redirectCode;
    std::string                         root;
    bool                                directoryListing;
    std::string                         defaultFile;
    bool                                uploadsAllowed;
    std::string                         uploadLocation;
    std::map<std::string, std::string>  cgiPaths;
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


auto    parseServerBlock(std::ifstream& inFile) -> std::expected<ServerBlock, std::string>;
auto    parseLocation(std::ifstream& inFile) -> std::expected<Location, std::string>;
auto    split(std::string line) -> std::expected<std::vector<std::string>, std::string>;

#endif // CONFIGPARSING_HPP
