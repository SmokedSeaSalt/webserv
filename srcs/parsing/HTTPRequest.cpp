#include "HTTPRequest.hpp"

auto HTTPRequest::getMessage() -> HTTPMessage
{
    return this->message_;
}

auto HTTPRequest::newData(std::string data) -> std::expected<ResponseStatusCode, ResponseStatusCode>
{
    this->buffer_ += data;

    std::string::size_type pos = this->buffer_.find(this->delimiter_);
    while (pos != std::string::npos)
    {

        switch (this->state_)
        {
        case (RequestState::kStartLine):
        {
            const auto ret = parseStartLine(this->buffer_.substr(0, pos));
            if (!ret.has_value())
                return std::unexpected(ret.error());

            this->state_ = RequestState::kHeaders;
            break;
        }
        case (RequestState::kHeaders):
        {
            if (pos == 0) // indicates last "\r\n"
            {
                if (this->expectBody())
                    this->state_ = RequestState::kBody;
                else
                    this->state_ = RequestState::KDone;
                break;
            }
            const auto ret = parseHeader(this->buffer_.substr(0, pos));
            if (!ret.has_value())
                return std::unexpected(ret.error());

            break;
        }
        case (RequestState::kBody):
        {
            // TODO make this better
            parseBody(this->buffer_.substr(0, pos));
            break;
        }
        case (RequestState::KDone):
        {
            // TODO
            // should never get here? return error too much data?
            break;
        }
        }
        this->buffer_.erase(0, pos + this->delimiter_.size());
        pos = this->buffer_.find(this->delimiter_);
    }

    return ResponseStatusCode::kOK;
}

auto HTTPRequest::parseStartLine(std::string line) -> std::expected<size_t, ResponseStatusCode>
{
    auto pos1 = line.find(' ');
    if (pos1 == std::string::npos)
        return std::unexpected(ResponseStatusCode::kBadRequest);
    auto pos2 = line.find(' ', pos1 + 1);
    if (pos2 == std::string::npos)
        return std::unexpected(ResponseStatusCode::kBadRequest);

    if (line.find(' ', pos2 + 1) != std::string::npos)
        return std::unexpected(ResponseStatusCode::kBadRequest);

    this->message_.method = line.substr(0, pos1);
    auto ret              = validateMethod(this->message_.method);
    if (!ret.has_value())
        return std::unexpected(ret.error());

    this->message_.requestTarget = line.substr(pos1 + 1, pos2 - pos1 - 1);
    ret                          = validateRequestTarget(this->message_.requestTarget);
    if (!ret.has_value())
        return std::unexpected(ret.error());

    this->message_.protocol = line.substr(pos2 + 1);
    ret                     = validateProtocol(this->message_.protocol);
    if (!ret.has_value())
        return std::unexpected(ret.error());

    return 1;
}

// split into key:value
// make lowercase -> store in key
// trim whitespaces -> store in value
// validate if the header is valid
auto HTTPRequest::parseHeader(std::string line) -> std::expected<size_t, ResponseStatusCode>
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

auto HTTPRequest::parseBody(std::string line) -> std::expected<size_t, ResponseStatusCode>
{
    this->message_.body += line;
    return line.size();
}

auto HTTPRequest::expectBody() -> bool
{
    if (this->message_.headers.contains("transfer-encoding"))
    {
        if (this->message_.headers["transfer-encoding"][0] == "chunked")
            return true;
    }
    if (this->message_.headers.contains("content-length"))
    {
        if (this->message_.headers["content-length"][0] == "0")
            return false;
        return true;
    }
    return false;
}