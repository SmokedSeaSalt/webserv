#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "Client.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "configParsing.hpp"
#include "connection.hpp"

namespace Execution
{

auto executeNonCGI(Client& client) -> HTTPResponse;

auto checkRequestConfigCompliance(Client& client) -> ResponseStatusCode;
auto setupRequestForExecution(Client& client) -> std::expected<void, HTTPResponse>;
auto buildErrorResponse(Client& client, ResponseStatusCode statusCode) -> HTTPResponse;
auto processValidRequest(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto getRedirectResponse(Client& client) -> HTTPResponse;
auto validateContentType(Client& client);

auto processGet(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processGetDir(Client client, const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processGetFile(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>;

auto processHead(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processPost(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processDelete(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;

}; // namespace Execution

#endif // EXECUTION_HPP
