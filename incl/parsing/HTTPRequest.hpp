#ifndef HTTPREQUEST
#define HTTPREQUEST

#include "HTTPRules.hpp"
#include "parsing.hpp"
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
        auto getExpectedBodyLength() const -> size_t;
        auto resetMessage() -> void;

        auto newData(std::string data) -> std::expected<RequestState, ResponseStatusCode>;
        auto getState() const -> RequestState;

    private:
        RequestState state_ = RequestState::kStartLine;
        std::string  buffer_;
        HTTPMessage  message_;
        BodyType     bodyType_;
        size_t       expectedBodyLength_;

        auto processStartLine() -> std::expected<bool, ResponseStatusCode>;
        auto processHeaders() -> std::expected<bool, ResponseStatusCode>;
        auto processBody() -> std::expected<bool, ResponseStatusCode>;

        auto parseStartLine(std::string line) -> std::expected<size_t, ResponseStatusCode>;
        auto parseHeader(std::string line) -> std::expected<size_t, ResponseStatusCode>;
        auto parseBody(std::string line) -> std::expected<size_t, ResponseStatusCode>;

        auto expectBody() -> std::expected<bool, ResponseStatusCode>;
};

#endif // HTTPREQUEST