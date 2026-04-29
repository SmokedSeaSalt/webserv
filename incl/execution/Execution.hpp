#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "configParsing.hpp"
#include "connection.hpp"
#include "Client.hpp"

namespace Execution
{

auto execute(Client& client) -> HTTPResponse;

auto checkRequestConfigCompliance(Client& client) -> ResponseStatusCode;
auto buildErrorResponse(Client& client, ResponseStatusCode statusCode) -> HTTPResponse;
auto processValidRequest(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto validateContentType(Client& client);

auto processGet(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processGetDir(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processGetFile(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>;

auto processHead(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processPost(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;
auto processDelete(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>;

}; // namespace Execution

#endif // EXECUTION_HPP
