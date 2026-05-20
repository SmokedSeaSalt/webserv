#include <expected>
#include <string>

std::string_view fileExtensionToContentType(std::string_view path);
std::string_view contentTypeToFileExtension(std::string_view path);

auto ecToResponseErrorStatusCode(std::error_code ec) -> ResponseStatusCode;

auto readFile(std::string pathString) -> std::expected<std::string, ResponseStatusCode>;
auto getAbsFilePath(HTTPRequest& request, bool useUploadStoreRoot = false) -> std::string;

auto createFileWithContent(std::string pathString, std::string body) -> std::expected<void, ResponseStatusCode>;

auto deleteFile(std::string pathString) -> std::expected<void, ResponseStatusCode>;