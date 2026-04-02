#ifndef MOCKHTTPREQUEST_HPP
#define MOCKHTTPREQUEST_HPP


#include <set>
#include <string>
#include <expected>
#include "parsing.hpp"
#include "HTTPRequest.hpp"

class HTTPRequest
{
    public:
        auto newData(std::string data) -> std::expected<void, std::string>;
        auto getBuffer() -> std::string;

    private:
        std::string buffer_;
};

#endif // MOCKHTTPREQUEST_HPP
