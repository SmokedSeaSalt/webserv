#include "parsing/HTTPRules.hpp"
#include "parsing/parsing.hpp"
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
        auto newData(std::string data) -> std::expected<void, std::string>;

    private:
        RequestState state_;
        std::string  buffer_;
        HTTPMessage  message_;

        auto parseStartLine(std::string line) -> std::expected<size_t, std::string>;
        auto parseHeader(std::string line) -> std::expected<size_t, std::string>;
        auto parseBody(std::string line) -> std::expected<size_t, std::string>;

        auto expectBody() -> bool;
};