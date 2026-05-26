#ifndef HTTPRESPONSE
#define HTTPRESPONSE

#include "HTTPRules.hpp"

enum class SendState
{
    kIdle,    // nothing prepared yet
    kReady,   // packet ready to start sending, but no data has been sent yet
    kSending, // partial send in progress
    kDone,    // fully sent
    kFailed   // send failed
};

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
        auto getStatusCode() -> ResponseStatusCode;

        auto setPacket(std::string packet) -> void;
        auto getRemainingPacket() -> std::string;
        auto getRemainingPacketLen() -> size_t;

        auto getPacket() -> std::string;

        auto incrementTotalBytesSent(size_t bytesSent) -> SendState;
        auto getTotalBytesSent() -> size_t;

        auto getSendState() const -> SendState;
        auto setSendState(SendState state) -> void;

        auto getBodyLen() const -> size_t;

        auto getKeepAlive() const -> bool;
        auto setKeepAlive(bool value) -> void;

    private:
        HTTPMessage        message_;
        ResponseStatusCode statusCode_ = ResponseStatusCode::kOK;
        std::string        packet_;
        size_t             totalBytesSent_ = 0;
        SendState          sendState_      = SendState::kIdle;
        bool               keepAlive_      = true;

        auto createFirstLine(ResponseStatusCode errorCode) const -> std::string;
        auto createHeaders() const -> std::string;
        auto createBody() const -> std::string;
        auto httpDate() const -> std::string;
};

#endif // HTTPRESPONSE