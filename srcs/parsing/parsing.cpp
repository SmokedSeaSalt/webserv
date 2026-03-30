#include <cctype>
#include "parsing"

bool is_tchar(char c)
{
    return (std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%' || c == '&' ||
           c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' || c == '^' || c == '_' ||
           c == '`' || c == '|' || c == '~');
}

auto validateMethod(std::string str) -> std::expected<bool, std::string>;
//https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#request_targets
auto validateRequestTarget(std::string str) -> std::expected<std::string, std::string>;
auto validateProtocol(std::string str) -> std::expected<bool, std::string>;
auto validateHeader(std::string str) -> std::expected<bool, std::string>;