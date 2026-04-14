#ifndef HTTPRESPONSE
#define HTTPRESPONSE

#include "HTTPRules.hpp"

class HTTPResponse : public HTTPRules
{
    public:
        auto createPacket() const -> std::string;
        auto createErrorPacket(ResponseStatusCode) const -> std::string;

        auto setHeader(std::string key, std::string value) -> void;
        auto addHeaderValue(std::string key, std::string value) -> void;
        auto setBody(std::string data) -> void;
        auto addBodyData(std::string data) -> void;

        auto setProtocol(std::string protocol) -> void;

    private:
        HTTPMessage message_;

        auto createFirstLine(ResponseStatusCode errorCode) const -> std::string;
        auto createHeaders() const -> std::string;
        auto createBody() const -> std::string;
        auto httpDate() const -> std::string;

};

#endif // HTTPRESPONSE