#include <expected>
#include <string>

std::string_view fileExtentionToContentType(std::string_view path);
std::string_view contentTypeToFileExtension(std::string_view path);

auto ecToResponseErrorStatusCode(std::error_code ec) -> ResponseStatusCode;

auto readFile(std::string pathString) -> std::expected<std::string, ResponseStatusCode>;
auto getAbsFilePath(std::string file) -> std::string;