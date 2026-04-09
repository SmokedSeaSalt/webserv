#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "configParsing.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class Execution
{
    public:
        Execution(Config config);

        auto execute(const HTTPRequest& request) -> HTTPResponse;

    private:
        Config config_;

        auto validateRequest(const HTTPRequest& request) -> ResponseStatusCode;
        auto buildErrorResponse(const HTTPRequest& request) -> HTTPResponse;
        auto processValidRequest(const HTTPRequest& request) -> HTTPResponse;


};

#endif // EXECUTION_HPP
