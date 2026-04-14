#ifndef HTTPRESPONSE
#define HTTPRESPONSE

#include "HTTPRules.hpp"

class HTTPResponse : public HTTPRules
{
    public:
        void               setResponseStatusCode(ResponseStatusCode statusCode);
        ResponseStatusCode getResponseStatusCode();
        void               setHeader(const std::string& header);
        std::string        getHeader();
        void               setBody(const std::string& body);
        std::string        getBody();

    private:
        std::string        header;
        std::string        body;
        ResponseStatusCode statusCode_;
};

#endif // HTTPRESPONSE