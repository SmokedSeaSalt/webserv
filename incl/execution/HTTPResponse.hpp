#ifndef HTTPRESPONSE
#define HTTPRESPONSE

#include "HTTPRules.hpp"

class HTTPResponse : public HTTPRules
{
    public:
        void               setResponseStatusCode(ResponseStatusCode statusCode);
        ResponseStatusCode getResponseStatusCode();

    private:
        ResponseStatusCode statusCode_;
};

#endif // HTTPRESPONSE