#include <string>
#include <expected>
#include "parsing/parsing.hpp"

enum class RequestState
{
    kStartLine,
    kHeaders,
    kBody,
    KDone
};

class HTTPRequest
{
    public:
       auto newData(std::string data) -> std::expected<size_t, std::string>;

    private:
        const std::string delimiter_ = "\\r\\n";

        RequestState state_;
        std::string buffer_;
        HTTPMessage message_;
    
        auto parseStartLine(std::string line) -> std::expected<size_t, std::string>;
        auto parseHeaders() -> std::expected<size_t, std::string>;
        auto parseBody() -> std::expected<size_t, std::string>;

};