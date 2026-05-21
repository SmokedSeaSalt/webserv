#include "CGIResponse.hpp"
#include "Execution.hpp"
#include "parsing.hpp"

// returns -1 on error
auto CGIResponse::parseResponse(std::string data) -> int
{
    if (data.find("\r\n\r\n") == std::string::npos)
        return -1;
    std::string headers = data.substr(0, data.find("\r\n\r\n") + 4);
    std::string body    = data.substr(data.find("\r\n\r\n") + 4);

    this->message_.body = body;

    while (true)
    {
        std::string::size_type pos = data.find(this->delimiter_);
        if (pos == std::string::npos)
            return -1;
        if (pos == 0)
            break;
        auto ret = parseHeader(data.substr(0, pos));
        if (!ret.has_value())
            return -1;
        data.erase(0, pos + this->delimiter_.size());
    }
    return 1;
}

// split into key:value
// make lowercase -> store in key
// trim whitespaces -> store in value
// validate if the header is valid
auto CGIResponse::parseHeader(std::string line) -> std::expected<size_t, ResponseStatusCode>
{
    auto colon_pos = line.find(":");
    if (colon_pos == std::string::npos || colon_pos == 0)
        return std::unexpected(ResponseStatusCode::kBadRequest);

    std::string key   = line.substr(0, colon_pos);
    key               = to_lower(key);
    std::string value = line.substr(colon_pos + 1);

    // trim value OWS (optional whitespace)
    size_t start = value.find_first_not_of(" \t");
    size_t end   = value.find_last_not_of(" \t");
    if (start == std::string::npos)
        value = "";
    else
        value = value.substr(start, end - start + 1);

    auto ret = validateHeader(key, value);
    if (!ret)
        return std::unexpected(ResponseStatusCode::kBadRequest);
    this->message_.headers[key].push_back(value);

    return line.size();
}

auto CGIResponse::getMessage() const -> HTTPMessage
{
    return this->message_;
}

auto CGIResponse::setMessage(HTTPMessage message) -> void
{
    this->message_ = message;
}