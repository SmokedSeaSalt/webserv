#include "parsing/HTTPRules.hpp"
#include <algorithm>
#include <cctype>

const std::string           HTTPRules::delimiter_        = "\r\n";
const std::set<std::string> HTTPRules::supportedMethods_ = {"GET", "HEAD", "POST", "DELETE"};
const std::string           HTTPRules::HTTPVersion_      = "HTTP/1.1";

auto HTTPRules::is_tchar(unsigned char c) -> bool
{
    return (std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%' || c == '&' ||
            c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' || c == '^' || c == '_' ||
            c == '`' || c == '|' || c == '~');
}

// all visable charachters. ascii value 32 -> 126 and tab
auto HTTPRules::is_vchar(unsigned char c) -> bool
{
    return ((c >= 0x21 && c <= 0x7e));
}

auto HTTPRules::is_obs_text(unsigned char c) -> bool
{
    return (c >= 0x80 && c <= 0xff);
}

auto HTTPRules::is_field_vchar(unsigned char c) -> bool
{
    return (is_vchar(c) || is_obs_text(c));
}

// does not really check for ABNF field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
// but it is way better to use in is_valid_field_value this way
auto HTTPRules::is_field_content(unsigned char c) -> bool
{
    return (is_vchar(c) || is_obs_text(c) || c == ' ' || c == '\t');
}

auto HTTPRules::is_field_value(std::string value) -> bool
{
    bool first    = is_field_vchar(*value.begin());
    bool last     = is_field_vchar(*value.end());
    bool validKey = std::all_of(value.begin(), value.end(), HTTPRules::is_field_content);

    return (first && last && validKey);
}

auto HTTPRules::validateMethod(std::string str) -> std::expected<bool, std::string>
{
    if (supportedMethods_.contains(str))
        return true;
    return std::unexpected("501 Not Implemented");
}

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#request_targets
auto HTTPRules::validateRequestTarget(std::string str) -> std::expected<std::string, std::string>
{
    // TODO
}

auto HTTPRules::validateProtocol(std::string str) -> std::expected<bool, std::string>
{
    if (str != HTTPVersion_)
        return std::unexpected("505 HTTP Version Not Supported");
}

auto HTTPRules::validateHeader(std::string key, std::string value) -> bool
{
    bool validKey = std::all_of(key.begin(), key.end(), HTTPRules::is_tchar);
    bool validValue = HTTPRules::is_field_value(value);

    return (validKey && validValue);
}