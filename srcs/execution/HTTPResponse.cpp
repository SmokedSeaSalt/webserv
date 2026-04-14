#include "HTTPResponse.hpp"

void HTTPResponse::setResponseStatusCode(ResponseStatusCode statusCode)
{
    statusCode_ = statusCode;
}

ResponseStatusCode HTTPResponse::getResponseStatusCode()
{
    return statusCode_;
}

void HTTPResponse::setHeader(const std::string& header)
{
    this->header = header;
}

std::string HTTPResponse::getHeader()
{
    return header;
}

void HTTPResponse::setBody(const std::string& body)
{
    this->body = body;
}

std::string HTTPResponse::getBody()
{
    return body;
}