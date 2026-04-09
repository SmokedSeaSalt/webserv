#ifndef HTTPRESPONSE
#define HTTPRESPONSE

#include "HTTPRules.hpp"

class HTTPResponse : public HTTPRules
{
    public:
        auto createPacket() -> std::string;
        auto createErrorPacket(ResponseStatusCode) -> std::string;

        auto setHeader(std::string key, std::string value);
        auto addHeaderValue(std::string key, std::string value);
        auto setBody(std::string data);

    private:
        HTTPMessage message_;

        auto createFirstLine() -> std::string;
        auto createHeaders() -> std::string;
        auto createBody() -> std::string;

        

};

#endif // HTTPRESPONSE