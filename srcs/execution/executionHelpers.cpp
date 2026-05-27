#include "Client.hpp"
#include "HTTPRules.hpp"
#include "InputArgs.hpp"
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

    bool fileExists = std::filesystem::exists(path, ec);
    if (!fileExists)
    {
        if (!ec)
            return std::unexpected(ResponseStatusCode::kNotFound);
        LOG(LogLevel::kDebug, "value: {} message: {}", ec.value(), ec.message());
        return std::unexpected(ecToResponseErrorStatusCode(ec));
    }

    std::uintmax_t size = std::filesystem::file_size(path, ec);
    if (ec)
    {
        LOG(LogLevel::kDebug, "value: {} message: {}", ec.value(), ec.message());
        return std::unexpected(ecToResponseErrorStatusCode(ec));
    }

    auto permissons = std::filesystem::status(path, ec).permissions();
    if (ec)
    {
        return std::unexpected(ecToResponseErrorStatusCode(ec));
    }

    if ((permissons & std::filesystem::perms::owner_read) == std::filesystem::perms::none &&
        (permissons & std::filesystem::perms::group_read) == std::filesystem::perms::none &&
        (permissons & std::filesystem::perms::others_read) == std::filesystem::perms::none)
    {
        return std::unexpected(ResponseStatusCode::kForbidden);
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

auto createFileWithContent(std::string pathString, std::string body) -> std::expected<void, ResponseStatusCode>
{
    std::error_code       ec;
    std::filesystem::path path{pathString};

    bool fileExists = std::filesystem::exists(path, ec);
    if (fileExists)
    {
        LOG(LogLevel::kDebug, "File {} already exists", path.string());
        return std::unexpected(ResponseStatusCode::kConflict);
    }

    std::filesystem::path parent = path.parent_path();
    if (!parent.empty())
    {
        bool fileParentExists = std::filesystem::exists(parent, ec);
        if (ec)
        {
            LOG(LogLevel::kDebug, "value: {} message: {}", ec.value(), ec.message());
            return std::unexpected(ecToResponseErrorStatusCode(ec));
        }
        if (!fileParentExists)
        {
            LOG(LogLevel::kDebug, "Subdirectory for {} does not exists", path.string());
            return std::unexpected(ResponseStatusCode::kNotFound);
        }

        auto permissions = std::filesystem::status(parent, ec).permissions();
        if (ec)
        {
            LOG(LogLevel::kDebug, "value: {} message: {}", ec.value(), ec.message());
            return std::unexpected(ecToResponseErrorStatusCode(ec));
        }

        if ((permissions & std::filesystem::perms::owner_write) == std::filesystem::perms::none &&
            (permissions & std::filesystem::perms::group_write) == std::filesystem::perms::none &&
            (permissions & std::filesystem::perms::others_write) == std::filesystem::perms::none)
        {
            LOG(LogLevel::kDebug, "File permissions for {} are not correct", path.string());
            return std::unexpected(ResponseStatusCode::kForbidden);
        }
    }

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out)
        return std::unexpected(ResponseStatusCode::kInternalServerError);

    out.write(body.data(), static_cast<std::streamsize>(body.size()));
    if (!out)
        return std::unexpected(ResponseStatusCode::kInternalServerError);

    out.close();
    if (!out)
        return std::unexpected(ResponseStatusCode::kInternalServerError);

    return {};
}

/// @brief deletes a file on the system. directories wont be deleted and will return an error.
/// @param pathString file path
/// @return void or an appropriate http status error code
auto deleteFile(std::string pathString) -> std::expected<void, ResponseStatusCode>
{
    std::error_code       ec;
    std::filesystem::path path{pathString};

    const bool isADirectory = std::filesystem::is_directory(path, ec);
    if (ec)
    {
        return std::unexpected(ecToResponseErrorStatusCode(ec));
    }
    if (isADirectory)
    {
        return std::unexpected(ResponseStatusCode::kMethodNotAllowed);
    }

    const bool removed = std::filesystem::remove(path, ec);
    if (ec)
    {
        return std::unexpected(ecToResponseErrorStatusCode(ec));
    }
    if (!removed)
    {
        return std::unexpected(ResponseStatusCode::kNotFound);
    }
    return {};
}

/// @brief request.pathAfterLocation should be set already
/// @param request
/// @return
auto getAbsFilePath(HTTPRequest& request, bool useUploadStoreRoot = false) -> std::string
{
    std::string root;

    if (useUploadStoreRoot)
        root = request.getLocation().uploadLocation;
    else
        root = request.getLocation().root;
    std::string pathAfterLocation = request.getMessage().pathAfterLocation;

    if (!pathAfterLocation.empty() && pathAfterLocation[0] == '/')
        pathAfterLocation = pathAfterLocation.substr(1);

    if (!InputArgs::args.relativePath.empty())
    {
        if (!root.empty() && root[0] == '/')
            root = root.substr(1);
        return (std::filesystem::path(InputArgs::args.relativePath) / root / pathAfterLocation).string();
    }
    else
    {
        return (std::filesystem::path(root) / pathAfterLocation).string();
    }
}