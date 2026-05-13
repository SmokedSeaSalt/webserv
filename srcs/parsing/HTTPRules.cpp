#include "HTTPRules.hpp"
#include <algorithm>
#include <cctype>

const std::string HTTPRules::delimiter_   = "\r\n";
const std::string HTTPRules::HTTPVersion_ = "HTTP/1.1";

auto HTTPRules::is_tchar(const unsigned char& c) -> bool
{
    return (std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%' || c == '&' ||
            c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' || c == '^' || c == '_' ||
            c == '`' || c == '|' || c == '~');
}

// all visable charachters. ascii value 32 -> 126 and tab
auto HTTPRules::is_vchar(const unsigned char& c) -> bool
{
    return ((c >= 0x21 && c <= 0x7e));
}

auto HTTPRules::is_obs_text(const unsigned char& c) -> bool
{
    return (c >= 0x80 && c <= 0xff);
}

auto HTTPRules::is_field_vchar(const unsigned char& c) -> bool
{
    return (is_vchar(c) || is_obs_text(c));
}

// does not really check for ABNF field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar ]
// but it is way better to use in is_valid_field_value this way
auto HTTPRules::is_field_content(const unsigned char& c) -> bool
{
    return (is_vchar(c) || is_obs_text(c) || c == ' ' || c == '\t');
}

// TODO test when empty. should be valid
auto HTTPRules::is_field_value(const std::string& value) -> bool
{
    bool first    = is_field_vchar(value.front());
    bool last     = is_field_vchar(value.back());
    bool validKey = std::all_of(value.begin(), value.end(), HTTPRules::is_field_content);

    return (first && last && validKey);
}

auto HTTPRules::is_unreserved(const unsigned char& c) -> bool
{
    return (std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~');
}

auto HTTPRules::is_sub_delims(const unsigned char& c) -> bool
{
    return (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' || c == '*' ||
            c == '+' || c == ',' || c == ';' || c == '=');
}

auto HTTPRules::is_hexdig(const unsigned char& c) -> bool
{
    return (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

// TODO test when empty. should be valid
auto HTTPRules::is_segment(const std::string& str) -> bool
{
    int insidePctEncoded = 0;

    for (char c : str)
    {
        if (insidePctEncoded > 0)
        {
            if (!is_hexdig(c))
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
auto HTTPRules::is_absolute_path(const std::string& str) -> bool
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

auto HTTPRules::is_query(const std::string& str) -> bool
{
    int insidePctEncoded = 0;

    for (char c : str)
    {
        if (insidePctEncoded > 0)
        {
            if (!is_hexdig(c))
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

auto HTTPRules::is_origin_form(const std::string& str) -> bool
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

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#request_targets
auto HTTPRules::validateRequestTarget(const std::string& str)
    -> std::expected<bool, ResponseStatusCode>
{
    if (!is_origin_form(str))
        return std::unexpected(ResponseStatusCode::kBadRequest);
    return true;
}

auto HTTPRules::validateProtocol(const std::string& str) -> std::expected<bool, ResponseStatusCode>
{
    if (str != HTTPVersion_)
        return std::unexpected(ResponseStatusCode::kHTTPVersionNotSupported);
    return true;
}

auto HTTPRules::validateHeader(const std::string& key, const std::string& value) -> bool
{
    bool validKey   = std::all_of(key.begin(), key.end(), HTTPRules::is_tchar);
    bool validValue = HTTPRules::is_field_value(value);

    return (validKey && validValue);
}

auto HTTPRules::statusCodeToString(ResponseStatusCode code) -> std::string
{
    auto it = kStatusCodeStrings.find(code);
    if (it != kStatusCodeStrings.end())
        return it->second;
    return "500 Internal Server Error"; // safe fallback
}