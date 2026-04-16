#include "HTTPRules.hpp"
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

/// @brief reads the contents from a file into a string
/// @param pathString the absolute path to the file you want to read.
/// @return string with file contents
auto readFile(std::string pathString) -> std::expected<std::string, ResponseStatusCode>
{
    std::error_code         ec;
    std::filesystem::path{pathString};

    std::filesystem::exists(path, ec);
    if (ec == std::errc::no_such_file_or_directory)
        return std::unexpected(ResponseStatusCode::kNotFound);

    std::uintmax_t  size = std::filesystem::file_size(path, ec);
    if (ec)
    {

        if (ec == std::errc::permission_denied)
            return std::unexpected(ResponseStatusCode::kForbidden);
    }
    std::string   content(size, '\0');
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::unexpected("Error: readFile");

    in.read(&content[0], size);
    if (!in)
        return std::unexpected("Error: readFile");
    return content;
}

auto getAbsFilePath(std::string file) -> std::string
{
    // todo create actual abs file path using root provided as cli arg
    return (/*megaglobal.root + */ file);
}