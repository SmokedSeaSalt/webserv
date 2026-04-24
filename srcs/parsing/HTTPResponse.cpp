#include "HTTPResponse.hpp"
#include "parsing.hpp"
#include <chrono>
#include <format>

auto HTTPResponse::createPacket(ResponseStatusCode statusCode) const -> std::string
{
    return (createFirstLine(statusCode) + createHeaders() + createBody());
}

auto HTTPResponse::createPacket() const -> std::string
{
    return (createFirstLine(this->statusCode_) + createHeaders() + createBody());
}

auto HTTPResponse::setHeader(std::string key, std::string value) -> std::expected<void, ResponseStatusCode>
{
    std::string keyLower = to_lower(key);
    if (!validateHeader(key, value))
        return (std::unexpected(ResponseStatusCode::kInternalServerError));
    if (this->message_.headers.contains(keyLower))
        this->message_.headers[keyLower].clear();
    else
        this->message_.headers[keyLower].push_back(value);

    return {};
}

auto HTTPResponse::addHeaderValue(std::string key, std::string value) -> std::expected<void, ResponseStatusCode>
{
    std::string keyLower = to_lower(key);
    if (!validateHeader(key, value))
        return (std::unexpected(ResponseStatusCode::kInternalServerError));
    this->message_.headers[keyLower].push_back(value);

    return {};
}

auto HTTPResponse::clearAllHeaders() -> void
{
    this->message_.headers.clear();
}

auto HTTPResponse::setBody(std::string data) -> std::expected<void, ResponseStatusCode>
{
    this->message_.body = data;

    return {};
}

auto HTTPResponse::addBodyData(std::string data) -> std::expected<void, ResponseStatusCode>
{
    this->message_.body += data;

    return {};
}

auto HTTPResponse::setProtocol(std::string protocol) -> std::expected<void, ResponseStatusCode>
{
    if (!validateProtocol(protocol))
        return (std::unexpected(ResponseStatusCode::kInternalServerError));
    this->message_.protocol = protocol;

    return {};
}

auto HTTPResponse::setStatusCode(ResponseStatusCode statusCode) -> void
{
    this->statusCode_ = statusCode;
}

auto HTTPResponse::createFirstLine(ResponseStatusCode errorCode) const -> std::string
{
    return (this->message_.protocol + " " + statusCodeToString(errorCode) + this->delimiter_);
}

auto HTTPResponse::createHeaders() const -> std::string
{
    std::string result;
    result += "server: webserv" + this->delimiter_;
    result += "date: " + httpDate() + this->delimiter_;

    for (const auto& [key, values] : this->message_.headers)
    {
        for (const auto& value : values)
        {
            result += std::format("{}: {}{}", key, value, this->delimiter_);
        }
    }
    return (result + this->delimiter_);
}

auto HTTPResponse::httpDate() const -> std::string
{
    auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    return std::format("{:%a, %d %b %Y %H:%M:%S GMT}", now);
}

auto HTTPResponse::createBody() const -> std::string
{
    return this->message_.body;
}

auto HTTPResponse::setPacket(std::string packet) -> void
{
    this->packet_         = std::move(packet);
    this->totalBytesSent_ = 0;
}

auto HTTPResponse::getRemainingPacket() -> std::string
{
    if (this->totalBytesSent_ >= this->packet_.size())
        return {};
    return this->packet_.substr(this->totalBytesSent_);
}

auto HTTPResponse::getRemainingPacketLen() -> size_t
{
    if (this->totalBytesSent_ >= this->packet_.size())
        return 0;
    return this->packet_.size() - this->totalBytesSent_;
}

auto HTTPResponse::getPacket() -> std::string
{
    return this->packet_;
}

auto HTTPResponse::incrementTotalBytesSent(size_t bytesSent) -> SendState
{
    this->totalBytesSent_ += bytesSent;
    if (this->totalBytesSent_ >= this->packet_.size())
    {
        this->totalBytesSent_ = this->packet_.size();
        sendState_            = SendState::kDone;
    }
    else
        sendState_ = SendState::kSending;
    return sendState_;
}

auto HTTPResponse::getTotalBytesSent() -> size_t
{
    return totalBytesSent_;
}

auto HTTPResponse::getSendState() const -> SendState
{
    return sendState_;
}

auto HTTPResponse::setSendState(SendState state) -> void
{
    sendState_ = state;
}

auto HTTPResponse::getBodyLen() const -> size_t
{
    return createBody().length();
}
