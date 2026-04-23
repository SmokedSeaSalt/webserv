#ifndef HTTPRESPONSE
#define HTTPRESPONSE

#include "HTTPRules.hpp"

class HTTPResponse : public HTTPRules
{
    public:
        auto createPacket(ResponseStatusCode statusCode) const -> std::string;
        auto createPacket() const -> std::string;

        auto setHeader(std::string key, std::string value) -> std::expected<void, ResponseStatusCode>;
        auto addHeaderValue(std::string key, std::string value) -> std::expected<void, ResponseStatusCode>;
        auto clearAllHeaders() -> void;
        auto setBody(std::string data) -> std::expected<void, ResponseStatusCode>;
        auto addBodyData(std::string data) -> std::expected<void, ResponseStatusCode>;

        auto setProtocol(std::string protocol) -> std::expected<void, ResponseStatusCode>;
        auto setStatusCode(ResponseStatusCode statusCode) -> void;


        auto isReadyToSend() -> bool;
        auto setReadyToSend() -> void;

    private:
        HTTPMessage        message_;
        ResponseStatusCode statusCode_ = ResponseStatusCode::kOK;
        std::string        packet;
        bool               readyToSend_ = false;

        auto createFirstLine(ResponseStatusCode errorCode) const -> std::string;
        auto createHeaders() const -> std::string;
        auto createBody() const -> std::string;
        auto httpDate() const -> std::string;
};

#endif // HTTPRESPONSE