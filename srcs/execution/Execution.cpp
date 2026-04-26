#include "Execution.hpp"
#include "executionHelpers.hpp"
#include <filesystem>

// Public Functions
Execution::Execution(Config config) : config_(config) {}

auto Execution::execute(const HTTPMessage& request) -> HTTPResponse
{
    HTTPResponse       response;
    ResponseStatusCode status = checkRequestConfigCompliance(request);

    if (status != ResponseStatusCode::kOK)
        return buildErrorResponse(request, status);
    // if (cgi)
    //      executeCGI
    // else non cgi below
    auto validRequestResult = processValidRequest(request);
    if (!validRequestResult.has_value())
        response = buildErrorResponse(request, validRequestResult.error()); // todo check this error handling
    else
        response = validRequestResult.value();

    response.setSendState(SendState::kReady);
    response.setHeader("content-length", std::to_string(response.getBodyLen()));

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

auto Execution::buildErrorResponse(const HTTPMessage& request, ResponseStatusCode statusCode) -> HTTPResponse
{
    (void)request;
    HTTPResponse response;

    response.setStatusCode(statusCode);
    // error code, defailt error page
    return response;
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
    else if (request.method == "DELETE")
    {
        auto processDeleteResult = processDelete(request);
        if (!processDeleteResult.has_value())
            return std::unexpected(processDeleteResult.error());
        httpResponse = processDeleteResult.value();
    }
    if (request.headers.contains("connection") && request.headers.at("connection")[0] == "close")
    {
        httpResponse.addHeaderValue("connection", "close");
        httpResponse.setKeepAlive(false);
    }
    else
    {
        httpResponse.addHeaderValue("connection", "keep-alive");
        httpResponse.setKeepAlive(true);
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
    HTTPResponse response;

    auto readFileResult = readFile(path);
    if (!readFileResult.has_value())
        return std::unexpected(readFileResult.error());
    response.addBodyData(readFileResult.value());
    response.addHeaderValue("content-type", std::string(fileExtensionToContentType(path)));

    return response;
}

auto Execution::processGet(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    std::string     path = getAbsFilePath(request.requestTarget);
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
    HTTPResponse response;
    std::string  path = getAbsFilePath(request.requestTarget);

    auto postFileResult = createFileWithContent(path, request.body);
    if (!postFileResult.has_value())
        return std::unexpected(postFileResult.error());
    // todo: any headers need to be set here?
    response.setStatusCode(ResponseStatusCode::kCreated);
    return response;
}

auto Execution::processDelete(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse    response;
    std::string     path = getAbsFilePath(request.requestTarget);
    std::error_code ec;

    auto deleteFileResult = deleteFile(path);
    if (!deleteFileResult.has_value())
        return std::unexpected(deleteFileResult.error());
    // todo: any headers need to be set here?
    response.setStatusCode(ResponseStatusCode::kNoContent);
    return response;
}
