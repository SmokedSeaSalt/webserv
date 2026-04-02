#include <vector>
#include <string>
#include <map>

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
    std::size_t                 maxBodySize;
    std::map<int, std::string>  defaultErrorPages; // <error code, path to default error page>
    
};

struct Config
{
    std::vector<ServerBlock>   serverBlocks; // interface:port pairs
};