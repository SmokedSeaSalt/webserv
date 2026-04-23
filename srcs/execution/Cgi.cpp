#include "Cgi.hpp"
#include "parsing.hpp"
#include <sys/socket.h>

std::set<std::string> Cgi::CgiTypes_ = {".php", ".sh"};

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
    for (std::string CgiType : Cgi::CgiTypes_)
    {
        if (segment.ends_with(CgiType))
            return true;
    }
    return false;
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
        // TODO: get host from client connection data
    }

    // TODO: REMOTE_ADDR -> get host from client data
    // TODO: SERVER_PORT -> get port from client data

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
    env_strings.push_back("PATH_INFO=" + pathInfo);

    // TODO ROOT + pathInfo
    // std::string pathTranslated = (getLocation().root + pathInfo);
    // env_strings.push_back("PATH_TRANSLATED=" + pathTranslated);

    // TODO: all headers except content-type and content-length should be converted to HTTP_HEADER

    return env_strings;
}




auto execute() -> void
{
    int fd[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
}
