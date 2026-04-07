#include "HTTPRules.hpp"
#include "parsing.hpp"
#include <expected>
#include <set>
#include <string>

#ifndef HTTPREQUEST
#define HTTPREQUEST

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
        auto getMessage() -> HTTPMessage;

        auto newData(std::string data) -> std::expected<void, std::string>;

        private:
        RequestState state_ = RequestState::kStartLine;
        std::string  buffer_;
        HTTPMessage  message_;

        auto parseStartLine(std::string line) -> std::expected<size_t, std::string>;
        auto parseHeader(std::string line) -> std::expected<size_t, std::string>;
        auto parseBody(std::string line) -> std::expected<size_t, std::string>;

        auto expectBody() -> bool;
};

#endif // HTTPREQUEST