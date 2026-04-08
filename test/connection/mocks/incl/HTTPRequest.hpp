#ifndef MOCKHTTPREQUEST_HPP
#define MOCKHTTPREQUEST_HPP

#include "HTTPRequest.hpp"
#include "parsing.hpp"
#include <expected>
#include <set>
#include <string>

class HTTPRequest
{
    public:
        auto newData(std::string data) -> std::expected<void, std::string>;
        auto getBuffer() -> std::string;

    private:
        std::string buffer_;
};

#endif // MOCKHTTPREQUEST_HPP
