#ifndef LOGGING_HPP
#define LOGGING_HPP

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

constexpr auto level_name(LogLevel level) -> std::string_view;

}

#endif // LOGGING_HPP