#include "HTTPResponse.hpp"
#include <format>
#include <chrono>
#include "parsing.hpp"

auto HTTPResponse::createPacket() const -> std::string
{
    return (createFirstLine(ResponseStatusCode::kOK) + createHeaders() + createBody());
}

auto HTTPResponse::createErrorPacket(ResponseStatusCode errorCode) const -> std::string
{
    return (createFirstLine(errorCode) + createHeaders() + createBody());
}

auto HTTPResponse::setHeader(std::string key, std::string value) -> void
{
    std::string keyLower = to_lower(key);
    if (this->message_.headers.contains(keyLower))
        this->message_.headers[keyLower].clear();
    else
        this->message_.headers[keyLower].push_back(value);
}

auto HTTPResponse::addHeaderValue(std::string key, std::string value) -> void
{
    std::string keyLower = to_lower(key);
    this->message_.headers[keyLower].push_back(value);
}

auto HTTPResponse::setBody(std::string data) -> void
{
    this->message_.body = data;
}

auto HTTPResponse::addBodyData(std::string data) -> void
{
    this->message_.body += data;
}

auto HTTPResponse::setProtocol(std::string protocol) -> void
{
    this->message_.protocol = protocol;
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
