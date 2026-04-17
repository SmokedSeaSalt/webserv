#include "HTTPRules.hpp"
#include <expected>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <system_error>

/// @brief maps std::error_code to ResponseStatusCode
/// @param ec an error std::error_condition. Do not pass an ec that is false
/// @return correspondomg ResponseStatusCode
auto ecToResponseErrorStatusCode(std::error_code ec) -> ResponseStatusCode
{
    std::map<std::error_condition, ResponseStatusCode> map{
        {std::errc::no_such_file_or_directory, ResponseStatusCode::kNotFound},
        {std::errc::permission_denied, ResponseStatusCode::kForbidden},
        {std::errc::is_a_directory, ResponseStatusCode::kForbidden},
        {std::errc::not_a_directory, ResponseStatusCode::kNotFound},
        {std::errc::filename_too_long, ResponseStatusCode::kURITooLong},
        {std::errc::too_many_files_open, ResponseStatusCode::kInternalServerError},
        {std::errc::too_many_files_open_in_system, ResponseStatusCode::kInternalServerError},
    };

    auto it = map.find(ec.default_error_condition());
    if (it != map.end())
        return it->second;

    return ResponseStatusCode::kInternalServerError;
}

/// @brief reads the contents from a file into a string
/// @param pathString the absolute path to the file you want to read.
/// @return string with file contents
auto readFile(std::string pathString) -> std::expected<std::string, ResponseStatusCode>
{
    std::error_code       ec;
    std::filesystem::path path{pathString};
    ResponseStatusCode    tmpCode;

    bool fileExists = std::filesystem::exists(path, ec);
    if (!fileExists)
    {
        if (!ec)
            return std::unexpected(ResponseStatusCode::kNotFound);
        tmpCode = ecToResponseErrorStatusCode(ec);
        // todo log("value: "ec.value() " message: " ec.message());
        return std::unexpected(tmpCode);
    }

    std::uintmax_t size = std::filesystem::file_size(path, ec);
    if (ec)
    {
        tmpCode = ecToResponseErrorStatusCode(ec);
        // todo log("value: "ec.value() " message: " ec.message());
        return std::unexpected(tmpCode);
    }

    std::string   content(size, '\0');
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        // log("ifstream could not be constructed with " + path);
        return std::unexpected(ResponseStatusCode::kInternalServerError);
    }

    in.read(&content[0], size);
    if (!in)
    {
        // log("read error: only " + is.gcount() + " bytes could be read";
        return std::unexpected(ResponseStatusCode::kInternalServerError);
    }
    in.close();
    return content;
}

auto getAbsFilePath(std::string file) -> std::string
{
    // todo create actual abs file path using root provided as cli arg
    return (/*megaglobal.root + */ file);
}

// is_a_directory
//