#ifndef LOGGING_HPP
#define LOGGING_HPP

#include "HTTPRules.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <print>
#include <source_location>
#include <string_view>

enum class LogLevel
{
    kSilent  = 0,
    kInfo    = 1,
    kVerbose = 2,
    kErrors  = 3,
    kDebug   = 4,
};

namespace Logging
{

struct Logger
{
        FILE*    file  = nullptr;
        LogLevel level = LogLevel::kInfo;

        ~Logger();
};

inline Logger g_logger;

auto level_name(LogLevel level) -> std::string_view;
auto init(const char* path, LogLevel level) -> void;

template <typename... Args>
auto log_impl(LogLevel level, std::source_location locaction, std::format_string<Args...> format, Args&&... args) -> void
{
    if (g_logger.level == LogLevel::kSilent)
        return;
    auto msg  = std::format(format, std::forward<Args>(args)...);
    auto now  = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    auto line = std::format("[{:%H:%M:%S} GMT] [{}] [{}:{}] {}", now, level_name(level), locaction.file_name(), locaction.line(), msg);

    try
    {
        std::println(stderr, "{}", line);
        if (g_logger.file)
            std::println(g_logger.file, "{}", line);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

} // namespace Logging

#define LOG(level, format, ...) \
    Logging::log_impl(level, std::source_location::current(), format __VA_OPT__(, ) __VA_ARGS__)

#endif // LOGGING_HPP

// helpers
std::string getHTTPMessageString(const HTTPMessage& msg);