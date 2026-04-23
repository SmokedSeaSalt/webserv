#ifndef PARSING
#define PARSING

#include <string>

auto to_lower(std::string str) -> std::string;
auto split(std::string line, char delimChar = ' ') -> std::expected<std::vector<std::string>, std::string>;
std::string stringTrim(const std::string& s);

#endif
