#include <expected>
#include <string>

auto readFile(std::string path) -> std::expected<std::string, std::string>;
auto getAbsFilePath(std::string file) -> std::string;