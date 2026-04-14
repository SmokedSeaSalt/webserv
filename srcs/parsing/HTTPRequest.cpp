#include "HTTPRequest.hpp"

auto HTTPRequest::getMessage() -> HTTPMessage
{
    return this->message_;
}

auto HTTPRequest::newData(std::string data) -> std::expected<ResponseStatusCode, ResponseStatusCode>
{
    this->buffer_ += data;
    
    while(true)
    {
        switch (this->state_)
        {
        case RequestState::kStartLine:
        {
            std::string::size_type pos = this->buffer_.find(this->delimiter_);
            if (pos == std::string::npos)
                return ResponseStatusCode::kOK;
            auto ret = parseStartLine(this->buffer_.substr(0, pos));
            if (!ret.has_value())
                return std::unexpected(ret.error());
            this->buffer_.erase(0, pos + this->delimiter_.size());

            this->state_ = RequestState::kHeaders;
            break;
        }
        
        case RequestState::kHeaders:
        {
            std::string::size_type pos = this->buffer_.find(this->delimiter_);
            if (pos == std::string::npos)
                return ResponseStatusCode::kOK;
            if (pos == 0)
            {
                if (this->expectBody())
                    this->state_ = RequestState::kBody;
                else
                    this->state_ = RequestState::KDone;
                this->buffer_.erase(0, pos + this->delimiter_.size());
                break;
            }
            auto ret = parseHeader(this->buffer_.substr(0, pos));
            if (!ret.has_value())
                return std::unexpected(ret.error());
            this->buffer_.erase(0, pos + this->delimiter_.size());
            
            break;
        }

        case RequestState::kBody:
        {
            this->message_.body += buffer_;
            if (this->bodyType_ == BodyType::kBytes)
            {
                return ResponseStatusCode::kOK;
            }
            if (this->bodyType_ == BodyType::kChunked)
            {
                return ResponseStatusCode::kOK;
            }

            break;
        }

        case RequestState::KDone:
        {
            if (this->buffer_.empty())
                return ResponseStatusCode::kOK;
            return std::unexpected(ResponseStatusCode::kBadRequest);
            
            break;
        }
        }
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
        {
            this->bodyType_ = BodyType::kChunked;
            return true;
        }
    }
    if (this->message_.headers.contains("content-length"))
    {
        if (this->message_.headers["content-length"][0] == "0")
        {
            this->bodyType_ = BodyType::kNone;
            return false;
        }
        this->bodyType_ = BodyType::kBytes;
        return true;
    }
    this->bodyType_ = BodyType::kNone;
    return false;
}