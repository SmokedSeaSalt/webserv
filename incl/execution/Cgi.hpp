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

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

enum class CgiState
{
    kCreateEnv,
    kExecute,
    kReceiveCGIResponse,
    kBuildHTTPResponse,
    KDone,
};

class Cgi
{
    public:
        auto createEnv(const HTTPRequest& request) -> void;
        auto newData(std::string data) -> void;
        auto execute() -> void;
        auto createPacket() -> std::string;

    private:
        CgiState     state_;
        HTTPResponse response_;
};

#endif // CGI_HPP