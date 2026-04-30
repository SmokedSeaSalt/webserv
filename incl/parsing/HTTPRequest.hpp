#ifndef HTTPREQUEST
#define HTTPREQUEST

#include "HTTPRules.hpp"
#include "configParsing.hpp"
#include <expected>
#include <set>
#include <string>

enum class RequestState
{
    kStartLine,
    kHeaders,
    kBody,
    KDone
};

enum class BodyType
{
    kNone,
    kChunked,
    kBytes,
};

class HTTPRequest : public HTTPRules
{
    public:
        auto getMessage() const -> HTTPMessage;
        auto setMessage(HTTPMessage message) -> void;

        auto getExpectedBodyLength() const -> size_t;
        auto resetMessage() -> void;

        auto newData(std::string data) -> std::expected<RequestState, ResponseStatusCode>;
        auto getState() const -> RequestState;

        auto setAbsoluteTarget(std::string) -> void;
        auto setpathAfterLocation(std::string) -> void;


        auto getServerBlock() const -> const Config::ServerBlock&;
        auto setServerBlock(const Config::ServerBlock&) -> void;

        auto getLocation() const -> const Config::Location&;
        auto setLocation(Config::Location&) -> void;

    private:
        RequestState        state_ = RequestState::kStartLine;
        std::string         buffer_;
        HTTPMessage         message_;
        BodyType            bodyType_;
        size_t              expectedBodyLength_;
        Config::ServerBlock serverBlock_;
        Config::Location    location_;

        auto processStartLine() -> std::expected<bool, ResponseStatusCode>;
        auto processHeaders() -> std::expected<bool, ResponseStatusCode>;
        auto processBody() -> std::expected<bool, ResponseStatusCode>;

        auto parseStartLine(std::string line) -> std::expected<size_t, ResponseStatusCode>;
        auto parseHeader(std::string line) -> std::expected<size_t, ResponseStatusCode>;
        auto parseBody(std::string line) -> std::expected<size_t, ResponseStatusCode>;

        auto expectBody() -> std::expected<bool, ResponseStatusCode>;
};

#endif // HTTPREQUEST