#include "Execution.hpp"
#include "Client.hpp"
#include "configParsing.hpp"
#include "configUtils.hpp"
#include "executionHelpers.hpp"
#include "logging.hpp"
#include <filesystem>
#include <string>

namespace Execution
{

auto execute(Client& client) -> HTTPResponse
{
    HTTPResponse response;

    client.request.setAbsoluteTarget(getAbsFilePath(client.request.getMessage().requestTarget));

    auto serverBlock = getServerBlock(Config::config, client.listenSocketIpPortPair);
    if (!serverBlock.has_value())
    {
        // todo log
        return buildErrorResponse(client, ResponseStatusCode::kInternalServerError);
    }
    client.request.setServerBlock(serverBlock.value());

    auto location = getLocation(serverBlock.value(), client.request.getMessage().requestTarget);
    if (!location.has_value())
    {
        // todo log
        return buildErrorResponse(client, ResponseStatusCode::kInternalServerError);
    }
    client.request.setLocation(location.value());

    ResponseStatusCode status = checkRequestConfigCompliance(client);

    if (status != ResponseStatusCode::kOK)
        return buildErrorResponse(client, status);
    // if (cgi)
    //      executeCGI
    // else non cgi below
    auto validRequestResult = processValidRequest(client);
    if (!validRequestResult.has_value())
        response = buildErrorResponse(client, validRequestResult.error()); // todo check this error handling
    else
        response = validRequestResult.value();

    response.setSendState(SendState::kReady);

    return response;
}

// Private Functions
// validate if request complies with config
auto checkRequestConfigCompliance(Client& client) -> ResponseStatusCode
{
    HTTPMessage requestMessage = client.request.getMessage();
    // Server block checks
    ResponseStatusCode tmpStatus = checkContentLength(client.request);
    if (tmpStatus != ResponseStatusCode::kOK)
    {
        LOG(LogLevel::kInfo, "Client at fd={}, content length check failed.", client.socketfd);
        return tmpStatus;
    }
    tmpStatus = checkLocationCompliance(client.request);
    if (tmpStatus != ResponseStatusCode::kOK)
        return tmpStatus;

    return ResponseStatusCode::kOK;
}

auto buildErrorResponse(Client& client, ResponseStatusCode statusCode) -> HTTPResponse
{
    (void)client;
    HTTPResponse response;

    response.setStatusCode(statusCode);
    // error code, defailt error page
    response.setHeader("content-length", std::to_string(response.getBodyLen()));

    return response;
}

auto processValidRequest(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse httpResponse;
    HTTPMessage  request = client.request.getMessage();

    if (request.method == "GET")
    {
        auto processGetResult = processGet(client);
        if (!processGetResult.has_value())
            return std::unexpected(processGetResult.error());
        httpResponse = processGetResult.value();
    }
    else if (request.method == "HEAD")
    {
        auto processHeadResult = processHead(client);
        if (!processHeadResult.has_value())
            return std::unexpected(processHeadResult.error());
        httpResponse = processHeadResult.value();
    }
    else if (request.method == "POST")
    {
        auto processPostResult = processPost(client);
        if (!processPostResult.has_value())
            return std::unexpected(processPostResult.error());
        httpResponse = processPostResult.value();
    }
    else if (request.method == "DELETE")
    {
        auto processDeleteResult = processDelete(client);
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

auto processGetDir(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    (void)path;
    return std::unexpected(ResponseStatusCode::kNotImplemented);
}

auto processGetFile(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse response;

    auto readFileResult = readFile(path);
    if (!readFileResult.has_value())
        return std::unexpected(readFileResult.error());
    response.addBodyData(readFileResult.value());
    response.addHeaderValue("content-type", std::string(fileExtensionToContentType(path)));

    return response;
}

auto processGet(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    std::string     path = client.request.getMessage().absoluteRequestTarget;
    std::error_code ec;
    HTTPResponse    response;

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
        response = processGetFileResult.value();
        response.setHeader("content-length", std::to_string(response.getBodyLen()));
        return response;
    }
}

auto processHead(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse response;

    auto processGetResult = processGet(client);
    if (!processGetResult.has_value())
        return std::unexpected(processGetResult.error());
    response = processGetResult.value();
    response.setBody("");
    return response;
}

auto processPost(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse response;
    std::string  path = client.request.getMessage().absoluteRequestTarget;

    auto postFileResult = createFileWithContent(path, client.request.getMessage().body);
    if (!postFileResult.has_value())
        return std::unexpected(postFileResult.error());
    // todo: any headers need to be set here?
    response.setStatusCode(ResponseStatusCode::kCreated);
    response.setHeader("content-length", std::to_string(response.getBodyLen()));
    return response;
}

auto processDelete(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse    response;
    std::string     path = client.request.getMessage().absoluteRequestTarget;
    std::error_code ec;

    auto deleteFileResult = deleteFile(path);
    if (!deleteFileResult.has_value())
        return std::unexpected(deleteFileResult.error());
    // todo: any headers need to be set here?
    response.setStatusCode(ResponseStatusCode::kNoContent);
    response.setHeader("content-length", std::to_string(response.getBodyLen()));
    return response;
}

} // namespace Execution