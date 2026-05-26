#include "Cgi.hpp"
#include "CGIResponse.hpp"
#include "ConnectionManager.hpp"
#include "Execution.hpp"
#include "InputArgs.hpp"
#include "configUtils.hpp"
#include "connection.hpp"
#include "executionHelpers.hpp"
#include "logging.hpp"
#include "parsing.hpp"
#include <string>
#include <sys/socket.h> //for socketpair
#include <unistd.h>     //for dup2, close

Cgi::Cgi() : state_(CgiState::KDone)
{
    this->bodyToCgiBytesSend_ = 0;
}

auto Cgi::getState() -> CgiState
{
    return this->state_;
}

auto Cgi::handleEvent(const epoll_event& epollEvent) -> HandleEventResult
{
    int      fd     = epollEvent.data.fd;
    uint32_t events = epollEvent.events;

    switch (this->state_)
    {
    case CgiState::kSendingBody:
    {
        if (!(events & EPOLLOUT))
            break;
        ssize_t bytesSend = ConnectionManager::handleSendingEvent(fd, this->bodyToCgi_.substr(this->bodyToCgiBytesSend_));

        if (bytesSend < 0)
        {
            LOG(LogLevel::kErrors, "Error during CGI send on fd:{}", fd);
            return HandleEventResult::kError;
        }
        this->bodyToCgiBytesSend_ += bytesSend;
        if (bytesSend == 0 || this->bodyToCgiBytesSend_ >= this->bodyToCgi_.size())
        {
            LOG(LogLevel::kInfo, "CGI finished sending on fd:{}", fd);
            this->state_ = CgiState::kReceiveCGIResponse;
            shutdown(fd, SHUT_WR);
            ConnectionManager::changeCGIConnectionToRead(fd);
            break;
        }
        break;
    }
    case CgiState::kReceiveCGIResponse:
    {
        LOG(LogLevel::kDebug, "kReceiveCGIResponse event received, events={:#x}, fd={}", events, fd);
        if (!(events & EPOLLIN))
            break;
        auto handleReceivingEventResult = ConnectionManager::handleReceivingEvent(fd);
        if (std::get<1>(handleReceivingEventResult) < 0)
        {
            // error happened but it might be EAGAIN
            LOG(LogLevel::kDebug, "recv() returned < 0 at fd: {}, closing connection.", fd);
            ConnectionManager::closeConnection(this->fd_);
            return HandleEventResult::kError;
        }
        this->cgiResponse_ += std::get<0>(handleReceivingEventResult);
        if (std::get<1>(handleReceivingEventResult) == 0)
        {
            LOG(LogLevel::kInfo, "CGI finished receiving on fd:{}", fd);
            LOG(LogLevel::kDebug, "Data received on fd:{} = {}", fd, this->cgiResponse_);
            // EOF happened
            // build actual http response for client
            ConnectionManager::getClient(this->fd_)->setResponse(this->createResponse(ConnectionManager::getClient(this->fd_)));
            ConnectionManager::closeConnection(this->fd_);
            this->state_ = CgiState::KDone;
            break;
        }
        break;
    }
    case CgiState::KDone:
    {
        return HandleEventResult::kSuccess;
    }
    }
    return HandleEventResult::kSuccess;
}

auto Cgi::isRequestTargetCgi(const std::string target, const Config::Location& location) -> bool
{
    std::string URI;
    if (target.find("?") == std::string::npos)
        URI = target;
    else
        URI = target.substr(0, target.find("?"));
    auto targetSegments = split(URI, '/');
    for (std::string& segment : targetSegments.value())
    {
        if (endsInCgi(segment, location))
            return true;
    }
    return false;
}

auto Cgi::endsInCgi(const std::string& segment, const Config::Location& location) -> bool
{
    auto& cgiTypes = location.cgiPaths;
    for (auto& [CgiType, CgiPath] : cgiTypes)
    {
        if (segment.ends_with(CgiType))
            return true;
    }
    return false;
}

auto Cgi::getInterpreterPath(std::string path, const Config::Location& location) -> std::string
{
    auto& cgiTypes = location.cgiPaths;
    for (auto& [CgiType, CgiPath] : cgiTypes)
    {
        if (path.ends_with(CgiType))
            return CgiPath;
    }
    return "";
}

auto Cgi::createEnv(const HTTPRequest& request, std::shared_ptr<Client> client) -> std::vector<std::string>
{

    std::vector<std::string> env_strings;
    env_strings.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env_strings.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env_strings.push_back("SERVER_SOFTWARE=webserv/1.0");
    env_strings.push_back("REMOTE_IDENT=");
    env_strings.push_back("REMOTE_USER=");
    env_strings.push_back("REQUEST_METHOD=" + request.getMessage().method);

    if (request.getMessage().headers.contains("content-type"))
        env_strings.push_back("CONTENT_TYPE=" + request.getMessage().headers["content-type"][0]);
    else
        env_strings.push_back("CONTECT_TYPE=application/octet-stream");
    if (request.getMessage().headers.contains("content-length"))
        env_strings.push_back("CONTENT_LENGTH=" + request.getMessage().headers["content-length"][0]);
    else
        env_strings.push_back("CONTECT_LENGTH=");

    // SERVER_NAME -> get from host header (untill : PORT) or from client connection data
    if (request.getMessage().headers.contains("host"))
    {
        std::string host = request.getMessage().headers["host"][0];
        if (host.find_first_of(':') != std::string::npos)
            env_strings.push_back("SERVER_NAME=" + host.substr(0, host.find_first_of(':')));
        else
            env_strings.push_back("SERVER_NAME=" + host);
    }
    else
    {
        env_strings.push_back("SERVER_NAME=" + get<std::string>(client->getListenSocketIpPortPair()));
    }
    env_strings.push_back("SERVER_PORT=" + std::to_string(get<int>(client->getListenSocketIpPortPair())));

    env_strings.push_back("REMOTE_ADDR=" + client->getHost());
    env_strings.push_back("REMOTE_HOST=" + client->getHost());

    std::string target = request.getMessage().requestTarget;
    std::string query  = "";
    if (target.find_first_of('?') != std::string::npos)
    {
        query = target.substr(target.find_first_of('?') + 1);
        target.erase(target.find_first_of('?'));
    }
    env_strings.push_back("QUERY_STRING=" + query);

    auto        targetSegments = split(target, '/');
    std::string scriptName     = "";
    std::string pathInfo       = "";
    bool        foundScript    = false;
    for (std::string& segment : targetSegments.value())
    {
        if (!foundScript)
        {
            if (!Cgi::endsInCgi(segment, request.getLocation()))
                scriptName += ("/" + segment);
            else
            {
                scriptName += ("/" + segment);
                foundScript = true;
            }
        }
        else
            pathInfo += ("/" + segment);
    }

    env_strings.push_back("SCRIPT_NAME=" + scriptName);
    this->scriptPath_ = scriptName;
    env_strings.push_back("PATH_INFO=" + pathInfo);

    std::string pathTranslated = (request.getLocation().root + pathInfo);
    env_strings.push_back("PATH_TRANSLATED=" + pathTranslated);

    for (auto& [key, value] : request.getMessage().headers)
    {
        if (key == "content-type" || key == "content-length")
            continue;
        env_strings.push_back(headerToEnvVar(key, value));
    }

    return env_strings;
}

auto Cgi::headerToEnvVar(std::string header, std::vector<std::string> value) -> std::string
{
    std::string envHeader;
    for (auto& c : header)
    {
        c = std::toupper(static_cast<unsigned char>(c));
        if (c == '-')
            c = '_';
    }
    envHeader.append("HTTP_" + header + "=");

    std::string envValue;
    for (std::string& entry : value)
    {
        if (!envValue.empty())
            envValue.append(", ");
        envValue.append(entry);
    }
    return envHeader + envValue;
}

auto Cgi::init(std::shared_ptr<Client> client) -> std::expected<int, ResponseStatusCode>
{
    // make sure everything is clean
    this->bodyToCgi_          = "";
    this->bodyToCgiBytesSend_ = 0;
    this->cgiResponse_        = "";

    // create socketpair
    int fd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1)
        return std::unexpected(ResponseStatusCode::kInternalServerError);
    this->fd_ = fd[0];

    this->bodyToCgi_                    = client->getRequest().getMessage().body;
    std::vector<std::string> envStrings = createEnv(client->getRequest(), client);
    Config::ServerBlock      block      = Config::getServerBlock(client->getListenSocketIpPortPair()).value();
    Config::Location         location   = Config::getLocation(block, this->scriptPath_).value();
    this->interpreterPath_              = getInterpreterPath(this->scriptPath_, location);
    if (this->interpreterPath_ == "")
        return std::unexpected(ResponseStatusCode::kNotImplemented);

    // do we have read access to the script
    std::error_code       ec;
    std::filesystem::path path{InputArgs::args.relativePath + this->scriptPath_};

    bool fileExists = std::filesystem::exists(path, ec);
    if (!fileExists)
    {
        if (!ec)
            return std::unexpected(ResponseStatusCode::kNotFound);
        return std::unexpected(ecToResponseErrorStatusCode(ec));
    }

    auto permissons = std::filesystem::status(path, ec).permissions();
    if (ec)
    {
        return std::unexpected(ecToResponseErrorStatusCode(ec));
    }

    if ((permissons & std::filesystem::perms::owner_read) == std::filesystem::perms::none &&
        (permissons & std::filesystem::perms::group_read) == std::filesystem::perms::none &&
        (permissons & std::filesystem::perms::others_read) == std::filesystem::perms::none)
    {
        return std::unexpected(ResponseStatusCode::kForbidden);
    }

    // add to epoll here
    auto ret = ConnectionManager::addCGIConnection(this->fd_, client);
    if (!ret.has_value())
        return std::unexpected(ResponseStatusCode::kInternalServerError);
    this->state_ = CgiState::kSendingBody;
    if (this->bodyToCgi_.size() == 0)
    {
        shutdown(this->fd_, SHUT_WR); // tell child stdin is closed
        this->state_ = CgiState::kReceiveCGIResponse;
        ConnectionManager::changeCGIConnectionToRead(this->fd_);
    }

    pid_t pid = fork();
    if (pid == -1)
        return std::unexpected(ResponseStatusCode::kInternalServerError);
    client->setCgiPID(pid);

    if (pid == 0)
    {
        // Child.
        if (dup2(fd[1], STDIN_FILENO) == -1 || dup2(fd[1], STDOUT_FILENO) == -1)
            std::exit(1); // error check? What will epoll do? //make static close and free function in connectionmanager

        close(fd[0]); // Close parent side.
        close(fd[1]); // dupped, so can close child side.

        // create char** envp
        std::vector<char*> envp;
        for (auto& s : envStrings)
            envp.push_back(s.data());
        envp.push_back(nullptr);

        std::string cdPath;
        std::string scriptName;
        if (scriptPath_.find_last_of('/') != std::string::npos)
        {
            cdPath = scriptPath_.substr(0, scriptPath_.find_last_of('/'));
            if (cdPath.starts_with('/') == true)
                cdPath.erase(0, 1);
            scriptName = scriptPath_.substr(scriptPath_.find_last_of('/') + 1);
        }

        // get interpeter and script
        std::vector<char*> argv;
        argv.push_back(interpreterPath_.data());
        argv.push_back(scriptName.data());
        argv.push_back(nullptr);

        chdir(cdPath.data());
        LOG(LogLevel::kInfo, "Executing script in child. interpeter: {}, script: {} , in folder: {}", argv[0], argv[1], getcwd(nullptr, 0));
        execve(argv[0], argv.data(), envp.data());
        LOG(LogLevel::kErrors, "Execve failed!");
        std::exit(1); // error check? What will epoll do? //make static close and free function in connectionmanager
    }
    else
    {
        // Parent
        close(fd[1]); // Close child side.
        return this->fd_;
    }
}

auto Cgi::createResponse(std::shared_ptr<Client> client) -> HTTPResponse
{
    CGIResponse cgiResponse;
    if (cgiResponse.parseResponse(this->cgiResponse_) == -1)
        return Execution::buildErrorResponse(*client, ResponseStatusCode::kInternalServerError);

    // create response
    HTTPResponse response;

    // set StatusCode
    if (!cgiResponse.getMessage().headers.contains("status"))
        response.setStatusCode(ResponseStatusCode::kOK);
    else
    {
        std::string statusCodeStr;
        int         statusCode;
        statusCodeStr = cgiResponse.getMessage().headers["status"][0].substr(0, 3);
        try
        {
            statusCode = std::stoi(statusCodeStr);
            response.setStatusCode(static_cast<ResponseStatusCode>(statusCode));
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return Execution::buildErrorResponse(*client, ResponseStatusCode::kInternalServerError);
        }
    }

    // copy headers to response
    for (const auto& [key, values] : cgiResponse.getMessage().headers)
    {
        response.addHeaderValue(key, values[0]);
    }

    // check if content-length header needs to be added
    if (!cgiResponse.getMessage().headers.contains("content-length"))
        response.addHeaderValue("content-length", std::to_string(cgiResponse.getMessage().body.size()));

    response.addBodyData(cgiResponse.getMessage().body);

    return response;
}