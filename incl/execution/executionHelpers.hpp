#include <expected>
#include <string>

std::string_view fileExtentionToContentType(std::string_view path);
std::string_view contentTypeToFileExtension(std::string_view path);


auto readFile(std::string path) -> std::expected<std::string, std::string>;
auto getAbsFilePath(std::string file) -> std::string;