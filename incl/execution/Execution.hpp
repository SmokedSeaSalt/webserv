#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "configParsing.hpp"

class Execution
{
    public:
        Execution(Config config);

        auto execute(const HTTPRequest& request) -> HTTPResponse;

    private:
        Config config_;

        auto checkRequestConfigCompliance(const HTTPRequest& request) -> ResponseStatusCode;
        auto buildErrorResponse(const HTTPRequest& request) -> HTTPResponse;
        auto processValidRequest(const HTTPRequest& request) -> HTTPResponse;
};

#endif // EXECUTION_HPP
