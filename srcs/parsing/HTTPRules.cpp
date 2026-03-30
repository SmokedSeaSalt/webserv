#include <cctype>
#include "parsing/HTTPRules.hpp"


const std::string HTTPRules::delimiter_ = "\r\n";
const std::set<std::string> HTTPRules::supportedMethods_ = {"GET", "HEAD", "POST", "DELETE"};
const std::string HTTPRules::HTTPVersion_ = "HTTP/1.1";

auto HTTPRules::is_tchar(char c) -> bool
{
    return (std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%' || c == '&' ||
           c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' || c == '^' || c == '_' ||
           c == '`' || c == '|' || c == '~');
}

//all visable charachters. ascii value 33 -> 126
auto HTTPRules::is_vchar(char c) -> bool
{
    return (c >= '!' && c <= '~');
}

auto HTTPRules::validateMethod(std::string str) -> std::expected<bool, std::string>
{
    if (supportedMethods_.contains(str))
        return true;
    return std::unexpected("501 Not Implemented");
}

//https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#request_targets
auto HTTPRules::validateRequestTarget(std::string str) -> std::expected<std::string, std::string>
{
//TODO
}

auto HTTPRules::validateProtocol(std::string str) -> std::expected<bool, std::string>
{
    if (str != HTTPVersion_)
        return std::unexpected("505 HTTP Version Not Supported");
}

auto HTTPRules::validateHeader(std::string key, std::string value) -> std::expected<bool, std::string>
{
//TODO
}