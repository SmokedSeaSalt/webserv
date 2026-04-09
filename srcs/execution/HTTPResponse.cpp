#include "HTTPResponse.hpp"

void HTTPResponse::setResponseStatusCode(ResponseStatusCode statusCode)
{
	statusCode_ = statusCode;
}

ResponseStatusCode HTTPResponse::getResponseStatusCode()
{
	return statusCode_;
}