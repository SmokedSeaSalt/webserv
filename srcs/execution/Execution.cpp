#include "Execution.hpp"
#include <filesystem>

// Public Functions
Execution::Execution(Config config) : config_(config) {}

auto Execution::execute(const HTTPMessage& request) -> HTTPResponse
{
    HTTPResponse response;
    response.setResponseStatusCode(checkRequestConfigCompliance(request));
    if (response.getResponseStatusCode() != ResponseStatusCode::kOK)
        response = buildErrorResponse(request);
    else
    {
        auto validRequest = processValidRequest(request);
        if (!validRequest.has_value())
            buildErrorResponse(request); // todo check this error handling
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

auto Execution::processValidRequest(const HTTPMessage& request) -> std::expected<HTTPResponse, std::string>
{
    HTTPResponse    httpResponse;
    if (request.method == "GET")
    {
        auto fileContents = readFileContents(request);
        if (!fileContents.has_value())
            return std::unexpected("Error: readFileContents failed");
    }
    return httpResponse;
}

auto Execution::readFileContents(const HTTPMessage& request) -> std::expected<std::string, std::string>
{
    std::string filePath = request.requestTarget;

    auto size = std::filesystem::file_size(filePath);
    std::string content(size, '\0');
    std::ifstream in(filePath);
    in.read(&content[0], size);
    return content;
}


