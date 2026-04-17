#include "Execution.hpp"
#include "executionHelpers.hpp"
#include <filesystem>

// Public Functions
Execution::Execution(Config config) : config_(config) {}

auto Execution::execute(const HTTPMessage& request) -> HTTPResponse
{
    HTTPResponse response;

    if (checkRequestConfigCompliance(request) != ResponseStatusCode::kOK)
        response = buildErrorResponse(request);
    else
    {
        auto validRequest = processValidRequest(request);
        if (!validRequest.has_value())
            return buildErrorResponse(request); // todo check this error handling
        response = validRequest.value();
    }

    // request.state = ;
    return response;
}

// Private Functions
// validate if request complies with config
auto Execution::checkRequestConfigCompliance(const HTTPMessage& request) -> ResponseStatusCode
{
    (void)request;
    // todo: validate request
    return ResponseStatusCode::kOK;
}

auto Execution::buildErrorResponse(const HTTPMessage& request) -> HTTPResponse
{
    (void)request;
    return {};
}

auto Execution::processValidRequest(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse httpResponse;
    if (request.method == "GET")
    {
        auto processGetResult = processGet(request);
        if (!processGetResult.has_value())
            return std::unexpected(processGetResult.error());
        httpResponse = processGetResult.value();
    }
    return httpResponse;
}

auto Execution::processGet(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse response;

    auto readFileResult = readFile(request.requestTarget);
    if (!readFileResult.has_value())
        return std::unexpected(readFileResult.error());
    // todo response.addHeaderValue("",);
    response.addBodyData(readFileResult.value());
    response.addHeaderValue("content-type", std::string(fileExtentionToContentType(request.requestTarget)));
    return response;
}
