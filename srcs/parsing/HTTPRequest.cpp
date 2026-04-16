#include "HTTPRequest.hpp"

auto HTTPRequest::getMessage() const -> HTTPMessage
{
    return this->message_;
}

auto HTTPRequest::getExpectedBodyLength() const -> size_t
{
    return this->expectedBodyLength_;
}

// Resets everything it had parsed, but leaves the buffer.
// This is to help implementing persistant HTTP/1.1 connections.
auto HTTPRequest::resetMessage() -> void
{
    this->state_ = RequestState::kStartLine;
    this->message_.body.clear();
    this->message_.headers.clear();
    this->message_.method.clear();
    this->message_.protocol.clear();
    this->message_.requestTarget.clear();
}

/// @brief Receives data from event handler.
/// Adds it to buffer and tries to parse as much as possible.
/// @param data New data to process.
/// @return Expected: current parsing state.
/// Unexpected: ResponseStatusCode.
auto HTTPRequest::newData(std::string data) -> std::expected<RequestState, ResponseStatusCode>
{
    this->buffer_ += data;

    while (true)
    {
        switch (this->state_)
        {
        case RequestState::kStartLine:
        {
            auto ret = processStartLine();
            if (!ret.has_value())
                return std::unexpected(ret.error());
            if (ret.value())
                return this->state_;
            break;
        }

        case RequestState::kHeaders:
        {
            auto ret = processHeaders();
            if (!ret.has_value())
                return std::unexpected(ret.error());
            if (ret.value())
                return this->state_;
            break;
        }

        case RequestState::kBody:
        {
            auto ret = processBody();
            if (!ret.has_value())
                return std::unexpected(ret.error());
            if (ret.value())
                return this->state_;
            break;
        }

        case RequestState::KDone:
        {
            if (this->buffer_.empty())
                return this->state_;
            return std::unexpected(ResponseStatusCode::kBadRequest);

            break;
        }
        }
    }

    return this->state_;
}

/// @brief
/// @return Expected -> false: newData should continue.
/// Expected -> false: newData should also return.
/// Unexpected -> ResponseStatusCode of error found.
auto HTTPRequest::processStartLine() -> std::expected<bool, ResponseStatusCode>
{
    std::string::size_type pos = this->buffer_.find(this->delimiter_);
    if (pos == std::string::npos)
        return true;
    auto ret = parseStartLine(this->buffer_.substr(0, pos));
    if (!ret.has_value())
        return std::unexpected(ret.error());
    this->buffer_.erase(0, pos + this->delimiter_.size());

    this->state_ = RequestState::kHeaders;

    return false;
}

/// @brief
/// @return Expected -> false: newData should continue.
/// Expected -> false: newData should also return.
/// Unexpected -> ResponseStatusCode of error found.
auto HTTPRequest::processHeaders() -> std::expected<bool, ResponseStatusCode>
{
    std::string::size_type pos = this->buffer_.find(this->delimiter_);
    if (pos == std::string::npos)
        return true;
    if (pos == 0)
    {
        auto ret = this->expectBody();
        if (!ret.has_value())
            return std::unexpected(ret.error());
        if (ret.value())
            this->state_ = RequestState::kBody;
        else
            this->state_ = RequestState::KDone;
        this->buffer_.erase(0, pos + this->delimiter_.size());
        return false;
    }
    auto ret = parseHeader(this->buffer_.substr(0, pos));
    if (!ret.has_value())
        return std::unexpected(ret.error());
    this->buffer_.erase(0, pos + this->delimiter_.size());

    return false;
}

/// @brief
/// @return Expected -> false: newData should continue.
/// Expected -> false: newData should also return.
/// Unexpected -> ResponseStatusCode of error found.
auto HTTPRequest::processBody() -> std::expected<bool, ResponseStatusCode>
{
    if (this->bodyType_ == BodyType::kBytes)
    {
        if (this->buffer_.size() + this->message_.body.size() <= this->expectedBodyLength_)
        {
            this->message_.body += buffer_;
            this->buffer_.erase();
            if (this->message_.body.size() == this->expectedBodyLength_)
                this->state_ = RequestState::KDone;
        }
        else
        {
            const auto lengthLeft = this->expectedBodyLength_ - this->message_.body.size();
            this->message_.body += buffer_.substr(0, lengthLeft);
            this->buffer_.erase(0, lengthLeft);
            this->state_ = RequestState::KDone;
        }

        return false;
    }
    if (this->bodyType_ == BodyType::kChunked)
    {
        size_t chunkSize;
        auto   pos1 = this->buffer_.find(this->delimiter_);
        auto   pos2 = this->buffer_.find(this->delimiter_, pos1 + 1);
        if (pos1 == std::string::npos || pos2 == std::string::npos)
            return true;
        try
        {
            chunkSize = std::stoul(this->buffer_.substr(0, pos1), nullptr, 16);
        }
        catch (const std::exception& e)
        {
            return std::unexpected(ResponseStatusCode::kBadRequest);
        }
        if (chunkSize == 0)
        {
            this->buffer_.erase(pos2 + this->delimiter_.size());
            this->state_ = RequestState::KDone;
            return true;
        }
        this->message_.body += buffer_.substr(pos1 + this->delimiter_.size(), pos2 - (pos1 + this->delimiter_.size()));
        this->buffer_.erase(0, pos2 + this->delimiter_.size());

        return false;
    }
    if (this->bodyType_ == BodyType::kNone)
    {
        //should never get here
        return false;
    }
    return false;

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

auto HTTPRequest::expectBody() -> std::expected<bool, ResponseStatusCode>
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
        try
        {
            this->expectedBodyLength_ = std::stoul(this->message_.headers["content-length"][0]);
        }
        catch (const std::exception& e)
        {
            return std::unexpected(ResponseStatusCode::kBadRequest);
        }

        this->bodyType_ = BodyType::kBytes;
        return true;
    }
    this->bodyType_ = BodyType::kNone;
    return false;
}