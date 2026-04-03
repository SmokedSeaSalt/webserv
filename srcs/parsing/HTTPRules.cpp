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

// TODO test when empty. should be valid
auto HTTPRules::is_field_value(std::string value) -> bool
{
    bool first    = is_field_vchar(*value.begin());
    bool last     = is_field_vchar(*value.end());
    bool validKey = std::all_of(value.begin(), value.end(), HTTPRules::is_field_content);

    return (first && last && validKey);
}

auto HTTPRules::is_unreserved(unsigned char c) -> bool
{
    return (std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~');
}

auto HTTPRules::is_sub_delims(unsigned char c) -> bool
{
    return (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' || c == '*' ||
            c == '+' || c == ',' || c == ';' || c == '=');
}

auto HTTPRules::is_hexdig(unsigned char c) -> bool
{
    return (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

// TODO test when empty. should be valid
auto HTTPRules::is_segment(std::string str) -> bool
{
    int insidePctEncoded = 0;

    for (char c : str)
    {
        if (insidePctEncoded > 0)
        {
            if (!is_hexdig(c))
                ;
            return false;
            insidePctEncoded--;
            continue;
        }
        if (c == '%')
        {
            insidePctEncoded = 2;
            continue;
        }
        if (!(is_unreserved(c) || is_sub_delims(c) || c == ':' || c == '@'))
            return false;
    }
    if (insidePctEncoded > 0)
        return false;
    return true;
}

// 1* ( "/" segment)
auto HTTPRules::is_absolute_path(std::string str) -> bool
{
    if (str.empty() || str.front() != '/')
        return false;

    size_t pos = 0;
    while (pos < str.size())
    {
        if (str[pos] != '/')
            return false;

        size_t      next    = str.find('/', pos + 1);
        std::string segment = str.substr(pos + 1, next - (pos + 1));
        if (!is_segment(segment))
            return false;
        pos = next;
    }
    return true;
}

auto HTTPRules::is_query(std::string str) -> bool
{
    int insidePctEncoded = 0;

    for (char c : str)
    {
        if (insidePctEncoded > 0)
        {
            if (!is_hexdig(c))
                ;
            return false;
            insidePctEncoded--;
            continue;
        }
        if (c == '%')
        {
            insidePctEncoded = 2;
            continue;
        }
        if (!(is_unreserved(c) || is_sub_delims(c) || c == ':' || c == '@' || c == '/' || c == '?'))
            return false;
    }
    if (insidePctEncoded > 0)
        return false;
    return true;
}

auto HTTPRules::is_origin_form(std::string str) -> bool
{
    auto pos = str.find("?");
    if (pos == std::string::npos)
    {
        if (is_absolute_path(str))
            return true;
        else
            return false;
    }

    if (!is_absolute_path(str.substr(0, pos - 1)))
        return false;
    if (!is_query(str.substr(pos + 1, str.length() - (pos + 1))))
        return false;
    return true;
}

auto HTTPRules::validateMethod(std::string str) -> std::expected<bool, std::string>
{
    if (supportedMethods_.contains(str))
        return true;
    return std::unexpected("501 Not Implemented");
}

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#request_targets
auto HTTPRules::validateRequestTarget(std::string str) -> std::expected<bool, std::string>
{
    if(!is_origin_form(str))
        return std::unexpected("400 Bad Request");
    return true;
}

auto HTTPRules::validateProtocol(std::string str) -> std::expected<bool, std::string>
{
    if (str != HTTPVersion_)
        return std::unexpected("505 HTTP Version Not Supported");
    return true;
}

auto HTTPRules::validateHeader(std::string key, std::string value) -> bool
{
    bool validKey   = std::all_of(key.begin(), key.end(), HTTPRules::is_tchar);
    bool validValue = HTTPRules::is_field_value(value);

    return (validKey && validValue);
}