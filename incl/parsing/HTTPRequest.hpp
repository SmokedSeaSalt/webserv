#include <set>
#include <string>
#include <expected>
#include "parsing/parsing.hpp"
#include "parsing/HTTPRules.hpp"

enum class RequestState
{
    kStartLine,
    kHeaders,
    kBody,
    KDone
};

class HTTPRequest: public HTTPRules
{
    public:
        auto newData(std::string data) -> std::expected<size_t, std::string>;

    private:
        RequestState state_;
        std::string buffer_;
        HTTPMessage message_;
    
        auto parseStartLine(std::string line) -> std::expected<size_t, std::string>;
        auto parseHeader(std::string line) -> std::expected<size_t, std::string>;
        auto parseBody() -> std::expected<size_t, std::string>;

        auto expectBody() -> bool;
};