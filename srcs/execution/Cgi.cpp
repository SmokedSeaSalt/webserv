#include "Cgi.hpp"
#include "ConnectionManager.hpp"
#include "connection.hpp"
#include "logging.hpp"
#include "parsing.hpp"
#include <sys/socket.h> //for socketpair
#include <unistd.h>     //for dup2, close

// this should be grabbed from config
std::map<std::string, std::string> Cgi::CgiTypes_ = {{".php", "/usr/bin/php"}, {".sh", "/usr/bin/sh"}};

Cgi::Cgi(Client& client) : client_(client), state_(CgiState::kInit)
{
    this->bodyToCgiBytesSend_ = 0;
}

auto Cgi::handleEvent(const epoll_event& epollEvent) -> HandleEventResult
{
    int      fd     = epollEvent.data.fd;
    uint32_t events = epollEvent.events;

    switch (this->state_)
    {
    case CgiState::kInit:
    {
        // should not get here.
    }
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
            break;
        }
    }
    case CgiState::kReceiveCGIResponse:
    {
        auto handleReceivingEventResult = ConnectionManager::handleReceivingEvent(fd);
        if (std::get<ssize_t>(handleReceivingEventResult) < 0)
        {
            // error happened but it might be EAGAIN
            LOG(LogLevel::kDebug, "recv() returned < 0 at fd: {}, closing connection.", fd);
            ConnectionManager::closeConnection(this->fd_); // TODO: how to handle this error? generate internal server error
            return HandleEventResult::kError;
        }
        if (std::get<ssize_t>(handleReceivingEventResult) == 0)
        {
            LOG(LogLevel::kInfo, "CGI finished receiving on fd:{}", fd);
            // EOF happened
            ConnectionManager::closeConnection(this->fd_);
            //build actual http response for client
            this->createResponse();
            this->state_ == CgiState::KDone;
        }
        this->cgiResponse_ += std::get<std::string>(handleReceivingEventResult);
        break;
    }
    case CgiState::KDone:
    {
    }

    return HandleEventResult::kSuccess;
    }
}

auto Cgi::isRequestTargetCgi(const std::string target) -> bool
{
    auto targetSegments = split(target, '/');
    for (std::string& segment : targetSegments.value())
    {
        if (endsInCgi(segment))
            return true;
    }
    return false;
}

auto Cgi::endsInCgi(const std::string& segment) -> bool
{
    for (auto& [CgiType, CgiPath] : Cgi::CgiTypes_)
    {
        if (segment.ends_with(CgiType))
            return true;
    }
    return false;
}

auto Cgi::getInterpreterPath(std::string path) -> std::string
{
    for (auto& [CgiType, CgiPath] : Cgi::CgiTypes_)
    {
        if (path.ends_with(CgiType))
            return CgiPath;
    }
    return "";
}

auto Cgi::createEnv(const HTTPRequest& request) -> std::vector<std::string>
{

    std::vector<std::string> env_strings;
    env_strings.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env_strings.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env_strings.push_back("SERVER_SOFTWARE=webserv/1.0");
    env_strings.push_back("REMOTE_IDENT=");
    env_strings.push_back("REMOTE_USER=");
    env_strings.push_back("REQUEST_METHOD=" + request.getMessage().method);

    if (request.getMessage().headers.contains("content-type"))
        env_strings.push_back("CONTENT_TYPE=" + request.getMessage().headers["contect-type"][0]);
    else
        env_strings.push_back("CONTECT_TYPE=application/octet-stream");
    if (request.getMessage().headers.contains("content-length"))
        env_strings.push_back("CONTENT_LENGTH=" + request.getMessage().headers["contect-length"][0]);
    else
        env_strings.push_back("CONTECT_LENGTH=");

    // SERVER_NAME -> get from host header (untill : PORT) or from client connection data
    if (request.getMessage().headers.contains("host"))
    {
        std::string host = request.getMessage().headers["host"][0];
        env_strings.push_back("SERVER_NAME=" + host.substr(0, host.find_first_of(':')));
    }
    else
    {
        env_strings.push_back("SERVER_NAME=" + get<std::string>(this->client_.getListenSocketIpPortPair()));
    }
    env_strings.push_back("SERVER_PORT=" + get<int>(this->client_.getListenSocketIpPortPair()));

    env_strings.push_back("REMOTE_ADDR=" + this->client_.getHost());
    env_strings.push_back("REMOTE_HOST=" + this->client_.getHost());

    std::string target = request.getMessage().requestTarget;
    std::string query  = target.substr(target.find_first_of('?'));
    target.erase(target.find_first_of('?'));
    env_strings.push_back("QUERY_STRING=" + query);

    auto        targetSegments = split(target, '/');
    std::string scriptName;
    std::string pathInfo;
    bool        foundScript = false;
    for (std::string& segment : targetSegments.value())
    {
        if (!foundScript)
        {
            if (!Cgi::endsInCgi(segment))
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

    // TODO ROOT + pathInfo
    // std::string pathTranslated = (getLocation().root + pathInfo);
    // env_strings.push_back("PATH_TRANSLATED=" + pathTranslated);

    for (auto& [key, value] : request.getMessage().headers)
    {
        if (key == "content-type" || key == "content-length")
            continue;
        env_strings.push_back(headerToEnvVar(key, value));
    }

    return env_strings;
}

static auto headerToEnvVar(std::string header, std::vector<std::string> value) -> std::string
{
    std::string envVar;
    for (auto& c : header)
    {
        c = std::toupper(static_cast<unsigned char>(c));
        if (c == '-')
            c = '_';
    }
    envVar.append("HTTP_" + header + "=");
    for (std::string entry : value)
    {
        envVar.append(entry);
    }
    return envVar;
}

auto Cgi::init() -> std::expected<int, HTTPResponse>
{
    // Todo do some more standard execution checking.
    this->bodyToCgi_                    = client_.getRequest().getMessage().body;
    std::vector<std::string> envStrings = createEnv(client_.getRequest()); // TODO get this acess to client request
    this->interpreterPath_              = getInterpreterPath(this->scriptPath_);
    if (this->interpreterPath_ == "")
        return; // TODO handle error; maybe place this within child if errors can be dealt with?

    int fd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1)
        return; // TODO Handle Error;
    pid_t pid = fork();

    if (pid == 0)
    {
        // Child.
        if (dup2(fd[1], STDIN_FILENO) == -1 || dup2(fd[1], STDOUT_FILENO) == -1)
            std::exit(1); // error check? What will epoll do?

        close(fd[0]); // Close parent side.
        close(fd[1]); // dupped, so can close child side.

        // create char** envp
        std::vector<char*> envp;
        for (auto& s : envStrings)
            envp.push_back(s.data());
        envp.push_back(nullptr);

        // get interpeter and script
        std::vector<char*> argv;
        argv.push_back(interpreterPath_.data());
        argv.push_back(scriptPath_.data());
        argv.push_back(nullptr);

        execve(argv[0], argv.data(), envp.data());
        LOG(LogLevel::kErrors, "Execve failed!");
        std::exit(1); // error check? What will epoll do?
    }
    else
    {
        // Parent
        close(fd[1]); // Close child side.
        this->fd_ = fd[0];
        ConnectionManager::addCGIConnection(this->fd_, this->client_); // Todo Error handling
        return this->fd_;
    }
}
