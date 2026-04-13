#include "HTTPResponse.hpp"
#include <format>
#include <chrono>

auto HTTPResponse::createPacket() -> std::string
{
    return (createFirstLine(ResponseStatusCode::kOK) + createHeaders() + createBody(ResponseStatusCode::kOK));
}

auto HTTPResponse::createErrorPacket(ResponseStatusCode errorCode) -> std::string
{
    //Allow header mandatory on 405 Method Not Allowed responses
    //WWW-Authenticate header mandatory on 401 Unauthorized responses
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

auto HTTPResponse::addBodydata(std::string data) -> void
{
    this->message_.body += data;
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
    //set some mandatory ones
    result += "Server: webserv" + this->delimiter_;
    //Date: timestamp.
    result += "Date: " + httpDate() + this->delimiter_;
    //Connection: close

    //Content-Length header required if there is a body and you're not using chunked transfer encoding
    //Content-Type header required if there is a body, otherwise clients don't know how to interpret it


    for (const auto& [key, values] : this->message_.headers)
    {
        for (const auto& value : values)
        {
            result += std::format("{}: {}{}", key, value, this->delimiter_);
        }
    }
    return (result + this->delimiter_);
}

auto HTTPResponse::httpDate() -> std::string
{
    auto now = std::chrono::utc_clock::now();
    return std::format("{:%a, %d %b %Y %H:%M:%S GMT}", now);
}

auto HTTPResponse::createBody(ResponseStatusCode errorCode) -> std::string
{
//check if chucked or not
//check for error code in config
//otherwise just return stored data
    return this->message_.body;
}
