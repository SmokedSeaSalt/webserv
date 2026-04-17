#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "configParsing.hpp"

class Execution
{
    public:
        Execution(Config config);

        auto execute(const HTTPMessage& request) -> HTTPResponse;

    private:
        Config config_;

        auto checkRequestConfigCompliance(const HTTPMessage& request) -> ResponseStatusCode;
        auto buildErrorResponse(const HTTPMessage& request) -> HTTPResponse;
        auto processValidRequest(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>;

        auto processGet(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>;
        auto processGetDir(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>;
        auto processGetFile(const std::string path) -> std::expected<HTTPResponse, ResponseStatusCode>;


        auto processHead(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>;
        auto processPost(const HTTPMessage& request) -> std::expected<HTTPResponse, ResponseStatusCode>;

};

#endif // EXECUTION_HPP
