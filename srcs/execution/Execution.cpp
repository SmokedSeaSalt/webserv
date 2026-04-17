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
    // error code, defailt error page
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
    else if (request.method == "HEAD")
    {
        auto processHeadResult = processHead(request);
        if (!processHeadResult.has_value())
            return std::unexpected(processHeadResult.error());
        httpResponse = processHeadResult.value();
    }
    else if (request.method == "POST")
    {
        auto processPostResult = processPost(request);
        if (!processPostResult.has_value())
            return std::unexpected(processPostResult.error());
        httpResponse = processPostResult.value();
    }
    return httpResponse;
}

auto Execution::processGetDir(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    (void)path;
    return std::unexpected(ResponseStatusCode::kNotImplemented);
}

auto Execution::processGetFile(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse    response;

    auto readFileResult = readFile(path);
    if (!readFileResult.has_value())
        return std::unexpected(readFileResult.error());
    response.addBodyData(readFileResult.value());
    response.addHeaderValue("content-type", std::string(fileExtentionToContentType(path)));

    return response;
}

auto Execution::processGet(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    std::string     path = request.requestTarget;
    std::error_code ec;

    bool fileExists = std::filesystem::exists(path, ec);
    if (!fileExists)
    {
        if (!ec)
            return std::unexpected(ResponseStatusCode::kNotFound);
        // todo log("value: "ec.value() " message: " ec.message());
        return std::unexpected(ecToResponseErrorStatusCode(ec));
    }

    bool isADirectory = std::filesystem::is_directory(path, ec);
    if (ec)
    {
        // todo log("value: "ec.value() " message: " ec.message());
        return std::unexpected(ecToResponseErrorStatusCode(ec));
    }
    if (isADirectory)
    {
        auto processGetDirResult = processGetDir(path);
        if (!processGetDirResult.has_value())
        {
            // todo log
            return std::unexpected(processGetDirResult.error());
        }
        return processGetDirResult.value();
    }
    else
    {
        
        auto processGetFileResult = processGetFile(path);
        if (!processGetFileResult.has_value())
        {
            // todo log
            return std::unexpected(processGetFileResult.error());
        }
        return processGetFileResult.value();
    }
}

auto Execution::processHead(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse response;

    auto processGetResult = processGet(request);
    if (!processGetResult.has_value())
        return std::unexpected(processGetResult.error());
    response = processGetResult.value();
    response.setBody("");
    return response;
}

auto Execution::processPost(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    (void)request;
    HTTPResponse response;

    return response;
}

