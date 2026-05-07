// implement execve behaviour
// check socketpair and how to implement
// plug this into epoll
// add logic to check wheter it is cgi or normal get/post
// add arguments to environment

// subject:
// Execution of CGI, based on file extension (for example .php). Here are some
// specific remarks regarding CGIs:
// ∗ Have a careful look at the environment variables involved in the web
// server-CGI communication. The full request and arguments provided by
// the client must be available to the CGI.
// ∗ The same applies to the output of the CGI. If no content_length is
// returned from the CGI, EOF will mark the end of the returned data.
// ∗ The CGI should be run in the correct directory for relative path file access.
// ∗ Your server should support at least one CGI (php-CGI, Python, and so
// forth).

#ifndef CGI_HPP
#define CGI_HPP

#include "Client.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include <map>
#include <string>

enum class CgiState
{
    kInit,
    kSendingBody,
    kReceiveCGIResponse,
    kBuildHTTPResponse,
    KDone,
};

class Cgi
{
    public:
        Cgi(Client& client);
        auto handleEvent(const epoll_event& epollEvent) -> HandleEventResult;
        auto init() -> std::expected<int, HTTPResponse>;
        auto createResponse() -> std::string;

        static auto Cgi::isRequestTargetCgi(const std::string target, const Config::Location& location) -> bool;

    private:
        Client&     client_;

        ssize_t     bodyToCgiBytesSend_;
        std::string bodyToCgi_;
        std::string cgiResponse_;
        CgiState    state_;
        int         fd_;
        std::string scriptPath_;
        std::string interpreterPath_;

        auto newData(std::string data) -> void;
        auto createEnv(const HTTPRequest& request) -> std::vector<std::string>;

        static auto Cgi::endsInCgi(const std::string& segment, const Config::Location& location) -> bool;
        static auto Cgi::getInterpreterPath(std::string path, const Config::Location& location) -> std::string;

        static std::map<std::string, std::string> CgiTypes_;
        static auto                               getInterpreterPath(std::string) -> std::string;
};

#endif // CGI_HPP