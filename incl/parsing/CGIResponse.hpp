#include "HTTPRules.hpp"

class CGIResponse : public HTTPRules
{
    public:
        auto parseResponse(std::string data) -> int;

        auto getMessage() const -> HTTPMessage;
        auto setMessage(HTTPMessage message) -> void;

    private:
        HTTPMessage message_;

        auto parseHeader(std::string line) -> std::expected<size_t, ResponseStatusCode>;
};
