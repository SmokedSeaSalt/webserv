#ifndef PARSING
#define PARSING

#include <expected>
#include <string>
#include <vector>

auto to_lower(std::string str) -> std::string;
auto split(std::string line, char delimChar = ' ') -> std::expected<std::vector<std::string>, std::string>;
auto stringTrim(const std::string& s) -> std::string;

#endif
