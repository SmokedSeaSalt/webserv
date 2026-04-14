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

class HTTPRequest : public HTTPRules
{
    public:
        auto getMessage() const -> HTTPMessage;
        auto setMessage() -> HTTPMessage;


        auto newData(std::string data) -> std::expected<ResponseStatusCode, ResponseStatusCode>;
        auto getState() const -> RequestState;


    private:
        RequestState state_ = RequestState::kStartLine;
        std::string  buffer_;
        HTTPMessage  message_;

        auto parseStartLine(std::string line) -> std::expected<size_t, ResponseStatusCode>;
        auto parseHeader(std::string line) -> std::expected<size_t, ResponseStatusCode>;
        auto parseBody(std::string line) -> std::expected<size_t, ResponseStatusCode>;

        auto expectBody() -> bool;
};

#endif // HTTPREQUEST