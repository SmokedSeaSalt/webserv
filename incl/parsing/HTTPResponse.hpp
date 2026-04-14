#ifndef HTTPRESPONSE
#define HTTPRESPONSE

#include "HTTPRules.hpp"

class HTTPResponse : public HTTPRules
{
    public:
        auto createPacket() -> std::string;
        auto createErrorPacket(ResponseStatusCode) -> std::string;

        auto setHeader(std::string key, std::string value) -> void;
        auto addHeaderValue(std::string key, std::string value) -> void;
        auto setBody(std::string data) -> void;
        auto addBodyData(std::string data) -> void;

        auto setProtocol(std::string protocol) -> void;

    private:
        HTTPMessage message_;

        auto createFirstLine(ResponseStatusCode errorCode) -> std::string;
        auto createHeaders() -> std::string;
        auto createBody() -> std::string;
        auto httpDate() -> std::string;

};

#endif // HTTPRESPONSE