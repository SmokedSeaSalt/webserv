#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "configParsing.hpp"

namespace Execution
{

auto execute(const HTTPMessage& request) -> HTTPResponse;

auto checkRequestConfigCompliance(const HTTPMessage& request) -> ResponseStatusCode;
auto buildErrorResponse(const HTTPMessage& request, ResponseStatusCode statusCode) -> HTTPResponse;
auto processValidRequest(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>;

auto processGet(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processGetDir(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processGetFile(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>;

auto processHead(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processPost(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processDelete(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>;

}; // namespace Execution

#endif // EXECUTION_HPP
