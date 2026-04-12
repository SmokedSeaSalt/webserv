#include "HTTPResponse.hpp"
#include <format>

auto HTTPResponse::createPacket() -> std::string
{

}

auto HTTPResponse::createErrorPacket(ResponseStatusCode errorCode) -> std::string
{
    return (createFirstLine(errorCode) + createHeaders() + createBody(errorCode));
}

auto HTTPResponse::setHeader(std::string key, std::string value) -> void
{
    if (this->message_.headers.contains(key))
        this->message_.headers[key][0] = value;
    else
        this->message_.headers[key].push_back(value);
}

auto HTTPResponse::addHeaderValue(std::string key, std::string value) -> void
{
    this->message_.headers[key].push_back(value);
}

auto HTTPResponse::setBody(std::string data) -> void
{
    this->message_.body = data;
}

auto HTTPResponse::setProtocol(std::string protocol) -> void
{
    this->message_.protocol = protocol;
}

auto HTTPResponse::createFirstLine(ResponseStatusCode errorCode) -> std::string
{
    return (this->message_.protocol + " " + statusCodeToString(errorCode) + this->delimiter_);
}

auto HTTPResponse::createHeaders() -> std::string
{
    std::string result;
    for (const auto& [key, values] : this->message_.headers)
    {
        for (const auto& value : values)
        {
            result += std::format("{}: {}{}", key, value, this->delimiter_);
        }
    }
    return (result + this->delimiter_);
}

auto HTTPResponse::createBody(ResponseStatusCode errorCode) -> std::string
{
//check if chucked or not
//check for error code in config
}
