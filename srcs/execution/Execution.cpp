#include "Execution.hpp"

// Public Functions
Execution::Execution(Config config) : config_(config) {}

auto Execution::execute(const HTTPRequest& request) -> HTTPResponse
{
    HTTPResponse response;
    response.setResponseStatusCode(checkRequestConfigCompliance(request));
    if (response.getResponseStatusCode() != ResponseStatusCode::kOK)
        response = buildErrorResponse(request);
    else
        response = processValidRequest(request);

    // request.state = ;
    return response;
}

// Private Functions
// validate if request complies with config
auto Execution::checkRequestConfigCompliance(const HTTPRequest& request) -> ResponseStatusCode
{
    // todo: validate request
    return ResponseStatusCode::kOK;
}

auto Execution::buildErrorResponse(const HTTPRequest& request) -> HTTPResponse
{
}

auto Execution::processValidRequest(const HTTPRequest& request) -> HTTPResponse
{
}
