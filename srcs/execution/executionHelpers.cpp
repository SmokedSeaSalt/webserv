#include <expected>
#include <filesystem>
#include <fstream>
#include <string>

auto readFile(std::string path) -> std::expected<std::string, std::string>
{
    auto          size = std::filesystem::file_size(path);
    std::string   content(size, '\0');
    std::ifstream in(path);
    in.read(&content[0], size);
    return content;
}

auto getAbsFilePath(std::string file) -> std::string
{
    // todo create actual abs file path using root provided as cli arg
    return (/*megaglobal.root + */ file);
}