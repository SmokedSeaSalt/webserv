#include "Execution.hpp"
#include "Client.hpp"
#include "InputArgs.hpp"
#include "configParsing.hpp"
#include "configUtils.hpp"
#include "executionHelpers.hpp"
#include "logging.hpp"
#include <filesystem>
#include <string>

namespace Execution
{

auto getPathAfterLocation(Client& client) -> std::string
{
    std::string        locationPath = client.getRequest().getLocation().pathPrefix;
    const std::string& target       = client.getRequest().getMessage().requestTarget;
    int                startIndex   = target.find(locationPath);
    if (startIndex != 0)
    {
        LOG(LogLevel::kInfo, "Can't get path after location client fd: {}", client.getSocketfd());
        return "";
    }
    return target.substr(locationPath.length());
}

auto setupRequestForExecution(Client& client) -> std::expected<void, HTTPResponse>
{
    auto serverBlock = Config::getServerBlock(client.getListenSocketIpPortPair());
    if (!serverBlock.has_value())
    {
        LOG(LogLevel::kDebug, "Server block not found client fd: {}", client.getSocketfd());
        return std::unexpected(buildErrorResponse(client, ResponseStatusCode::kInternalServerError));
    }
    client.getRequest().setServerBlock(serverBlock.value());

    auto location = getLocation(serverBlock.value(), client.getRequest().getMessage().requestTarget);
    if (!location.has_value())
    {
        LOG(LogLevel::kDebug, " not found client fd: {}", client.getSocketfd());
        return std::unexpected(buildErrorResponse(client, ResponseStatusCode::kInternalServerError));
    }
    client.getRequest().setLocation(location.value());

    client.getRequest().setpathAfterLocation(getPathAfterLocation(client));

    client.getRequest().setAbsoluteTarget(getAbsFilePath(client.getRequest()));

    return {};
}

auto execute(Client& client) -> HTTPResponse
{
    HTTPResponse response;
    HTTPMessage  requestMessage = client.getRequest().getMessage();

    auto setupResult = setupRequestForExecution(client);
    if (!setupResult.has_value())
        return setupResult.error();

    ResponseStatusCode status = checkRequestConfigCompliance(client);

    if (status != ResponseStatusCode::kOK)
        response = buildErrorResponse(client, status);
    else
    {
        // if (cgi)
        //      executeCGI
        // else non cgi below
        auto validRequestResult = processValidRequest(client);
        if (!validRequestResult.has_value())
            response = buildErrorResponse(client, validRequestResult.error()); // todo check this error handling
        else
            response = validRequestResult.value();
    }
    response.setSendState(SendState::kReady);

    if (requestMessage.headers.contains("connection") && requestMessage.headers.at("connection")[0] == "close")
    {
        response.addHeaderValue("connection", "close");
        response.setKeepAlive(false);
    }
    else
    {
        response.addHeaderValue("connection", "keep-alive");
        response.setKeepAlive(true);
    }

    return response;
}

// Private Functions
// validate if request complies with config
auto checkRequestConfigCompliance(Client& client) -> ResponseStatusCode
{
    HTTPMessage requestMessage = client.getRequest().getMessage();
    // Server block checks
    ResponseStatusCode tmpStatus = Config::checkContentLength(client.getRequest());
    if (tmpStatus != ResponseStatusCode::kOK)
    {
        LOG(LogLevel::kInfo, "Client at fd={}, content length check failed.", client.getSocketfd());
        return tmpStatus;
    }
    tmpStatus = Config::checkLocationCompliance(client.getRequest());
    if (tmpStatus != ResponseStatusCode::kOK)
        return tmpStatus;

    return ResponseStatusCode::kOK;
}

auto buildErrorResponse(Client& client, ResponseStatusCode statusCode) -> HTTPResponse
{
    HTTPResponse response;

    std::map<int, std::string> defaultErrorPages = client.getRequest().getServerBlock().defaultErrorPages;
    if (defaultErrorPages.count(static_cast<int>(statusCode)))
    {
        std::string relativePath = defaultErrorPages[static_cast<int>(statusCode)];
        if (relativePath.length() > 0 && relativePath[0] == '/')
            relativePath = relativePath.substr(1);
        std::string absolutePath = std::filesystem::path(InputArgs::args.relativePath) / relativePath;
        auto        res          = processGetFile(absolutePath);
        if (res.has_value())
        {
            response = res.value();
            response.setStatusCode(statusCode);
            return response;
        }
    }
    response.setStatusCode(statusCode);
    response.setHeader("content-length", std::to_string(response.getBodyLen())); // todo check does this overwrite anything?
    return response;
}

auto processValidRequest(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse httpResponse;
    HTTPMessage  request = client.getRequest().getMessage();

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

    return httpResponse;
}

auto processDirectoryListing(Client client) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse    response;
    std::string     path = client.getRequest().getMessage().absoluteRequestTarget;
    std::error_code ec;

    if (!std::filesystem::exists(path, ec) || !std::filesystem::is_directory(path, ec))
    {
        return std::unexpected(ResponseStatusCode::kNotFound);
    }

    std::string html = "<!DOCTYPE html>\n<html><head><title>Directory listing for " + path + "</title></head><body>";
    html += "<h1>Directory listing for " + path + "</h1><ul>";

    for (std::filesystem::directory_iterator it(path, ec), end; it != end; ++it)
    {
        if (ec)
        {
            return std::unexpected(ResponseStatusCode::kInternalServerError);
        }
        std::string name = it->path().filename().string();
        std::string href = name;
        if (it->is_directory())
        {
            href += "/";
            name += "/";
        }

        std::filesystem::file_time_type       lastWriteTime = it->last_write_time();
        std::chrono::system_clock::time_point lastWriteTimeSystemClock =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                lastWriteTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t lastWriteTimeTimeT = std::chrono::system_clock::to_time_t(lastWriteTimeSystemClock);
        std::string formattedTimeStr   = std::asctime(std::localtime(&lastWriteTimeTimeT));
        formattedTimeStr.pop_back(); // remove trailing newline

        std::string sizeStr = it->is_regular_file()
                                  ? std::to_string(it->file_size()) + " bytes"
                                  : "-";

        html += "<li><a href=\"" + href + "\">" + name + "</a>"
                                                         " &nbsp; " +
                sizeStr +
                " &nbsp; " + formattedTimeStr + "</li>";
    }
    html += "</ul></body></html>";

    response.setStatusCode(ResponseStatusCode::kOK);
    response.addHeaderValue("content-type", "text/html");
    response.setHeader("content-length", std::to_string(html.size()));
    response.setBody(html);

    return response;
}

auto processGetDir(Client client, const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse response;
    (void)path;
    Config::Location location = client.getRequest().getLocation();

    if (location.defaultFile != "")
    {
        client.getRequest().setpathAfterLocation(location.defaultFile);
        auto processGetFileResult = processGetFile(getAbsFilePath(client.getRequest()));
        if (!processGetFileResult.has_value())
        {
            // todo log
            LOG(LogLevel::kDebug, "default file: {} could not be read", location.defaultFile);
            return std::unexpected(processGetFileResult.error());
        }
        return processGetFileResult.value();
    }
    else if (location.directoryListing)
    {
        auto processDirectoryListingResult = processDirectoryListing(client);
        if (!processDirectoryListingResult.has_value())
        {
            // todo log
            LOG(LogLevel::kDebug, "Directory listing at: {} failed", client.getRequest().getMessage().absoluteRequestTarget);
            return std::unexpected(ResponseStatusCode::kInternalServerError);
        }
        return processDirectoryListingResult.value();
    }
    else
        return std::unexpected(ResponseStatusCode::kNotFound);
}

auto processGetFile(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse response;

    auto readFileResult = readFile(path);
    if (!readFileResult.has_value())
        return std::unexpected(readFileResult.error());
    response.addBodyData(readFileResult.value());
    response.addHeaderValue("content-type", std::string(fileExtensionToContentType(path)));
    response.setHeader("content-length", std::to_string(response.getBodyLen()));

    return response;
}

auto processGet(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    std::string     path = client.getRequest().getMessage().absoluteRequestTarget;
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
        auto processGetDirResult = processGetDir(client, path);
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
    HTTPResponse       response;
    const HTTPRequest& request = client.getRequest();
    std::string        path    = request.getMessage().absoluteRequestTarget;

    auto postFileResult = createFileWithContent(path, request.getMessage().body);
    if (!postFileResult.has_value())
        return std::unexpected(postFileResult.error());
    // todo: any headers need to be set here?
    response.setStatusCode(ResponseStatusCode::kCreated);
    response.setHeader("content-length", std::to_string(response.getBodyLen()));
    std::string pathAfterLocation = request.getMessage().pathAfterLocation;
    if (!pathAfterLocation.empty() && pathAfterLocation[0] == '/')
        pathAfterLocation = pathAfterLocation.substr(1);
    std::string locationValue = std::filesystem::path(request.getLocation().pathPrefix) / std::filesystem::path(pathAfterLocation).generic_string();
    if (!locationValue.empty() && locationValue[0] != '/')
        locationValue = '/' + locationValue;
    response.setHeader("location", locationValue);
    return response;
}

auto processDelete(Client& client) -> std::expected<HTTPResponse, ResponseStatusCode>
{
    HTTPResponse    response;
    std::string     path = client.getRequest().getMessage().absoluteRequestTarget;
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