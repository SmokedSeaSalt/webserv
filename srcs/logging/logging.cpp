#include <chrono>
#include <format>
#include <print>
#include <source_location>
#include <string_view>

namespace Logging
{

auto logfile(FILE* newFile = nullptr) -> FILE*
{
    static FILE* file = nullptr;
    if (newFile)
        file = newFile;
    return file;
}

constexpr auto level_name(LogLevel level) -> std::string_view
{
    switch (level)
    {
    case LogLevel::kInfo:
        return "INFO";
    case LogLevel::kVerbose:
        return "VERBOSE";
    case LogLevel::kErrors:
        return "ERROR ";
    case LogLevel::kDebug:
        return "DEBUG ";
    }
}

template <typename... Args>
auto log_impl(LogLevel level, std::source_location locaction, std::format_string<Args...> format, Args&&... args) -> void
{
    if (InputArgs::get().logLevel == LogLevel::kSilent)
        return;
    auto msg  = std::format(format, std::forward<Args>(args)...);
    auto now  = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    auto line = std::format("[{:%H:%M:%S}] [{}] [{}:{}] {}", now, level_name(level), locaction.file_name(), locaction.line(), msg);

    std::println("{}", line);
}

} // namespace Logging

#define LOG(level, format, ...) \
    Logging::log_impl(level, std::source_location::current(), format __VA_OPT__(, ) __VA_ARGS__)