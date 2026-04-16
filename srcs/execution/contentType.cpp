#include <string_view>
#include <unordered_map>

static const std::unordered_map<std::string_view, std::string_view> types = {
    // Images
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif", "image/gif"},
    {".webp", "image/webp"},
    {".svg", "image/svg+xml"},
    {".ico", "image/x-icon"},
    // Web
    {".html", "text/html; charset=utf-8"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    // Misc
    {".pdf", "application/pdf"},
    {".txt", "text/plain; charset=utf-8"},
    {".xml", "application/xml"},
};

std::string_view fileExtentionToContentType(std::string_view path)
{
    auto dot = path.rfind('.');
    if (dot == std::string_view::npos)
        return "application/octet-stream";

    auto ext = path.substr(dot);
    auto it  = types.find(ext);
    return it != types.end() ? it->second : "application/octet-stream";
}

// todo
std::string_view contentTypeToFileExtension(std::string_view path)
{
    return "TODO";
}
