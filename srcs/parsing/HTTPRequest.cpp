#include "parsing/HTTPRequest.hpp"

auto HTTPRequest::newData(std::string data) -> std::expected<size_t, std::string>
{
    this->buffer_ += data;

    std::string::size_type pos = this->buffer_.find(this->delimiter_);
    if (pos == std::string::npos)
        return (0);

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

        break;
    }
    case (RequestState::KDone):
    {

        break;
    }
    }

    this->buffer_.erase(pos + this->delimiter_.size());
    return pos;
}

auto HTTPRequest::parseStartLine(std::string line) -> std::expected<size_t, std::string> 
{
    auto pos1 = line.find(' ');
    if (pos1 == std::string::npos)
        return std::unexpected("400 Bad Request");
    auto pos2 = line.find(' ', pos1 + 1);
    if (pos2 == std::string::npos)
        return std::unexpected("400 Bad Request");

    if (line.find(' ', pos2 + 1) != std::string::npos)
        return std::unexpected("400 Bad Request");

    this->message_.method = line.substr(0, pos1);
    auto ret = validateMethod(this->message_.method);
    
    this->message_.requestTarget = line.substr(pos1 + 1, pos2 - pos1 - 1);
    auto ret = validateRequestTarget(this->message_.requestTarget);

    this->message_.protocol = line.substr(pos2 + 1);
    auto ret = validateRequestTarget(this->message_.protocol);

    return 1;
}

auto HTTPRequest::parseHeader(std::string line) -> std::expected<size_t, std::string> {}

auto HTTPRequest::parseBody() -> std::expected<size_t, std::string> {}

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