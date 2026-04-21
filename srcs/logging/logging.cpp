#include "logging.hpp"
#include <iostream>

namespace Logging
{

Logger::~Logger()
{
    if (file)
    {
        std::fflush(file);
        std::fclose(file);
        file = nullptr;
    }
}

auto init(const char* path, LogLevel level) -> void
{
    g_logger.level = level;
    if (path == nullptr)
        return;
    FILE* file = std::fopen(path, "a");
    if (file == nullptr)
    {
        std::cerr << "Opening logfile failed. Defaulting to logging to cout." << std::endl;
        return;
    }
    g_logger.file = file;
}

auto level_name(LogLevel level) -> std::string_view
{
    switch (level)
    {
    case LogLevel::kInfo:
        return "INFO";
    case LogLevel::kVerbose:
        return "VERBOSE";
    case LogLevel::kErrors:
        return "ERROR";
    case LogLevel::kDebug:
        return "DEBUG";
    default:
        return "";
    }
}

} // namespace Logging