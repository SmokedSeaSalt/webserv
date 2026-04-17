#ifndef INPUTARGUMENTS_HPP
#define INPUTARGUMENTS_HPP

#include <string>

namespace InputArgs
{

enum class LogLevel
{
    kSilent  = 0,
    kInfo    = 1,
    kVerbose = 2,
    kErrors  = 3,
    kDebug   = 4,
};

struct Args
{
        std::string relativePath;
        std::string logFile;
        std::string configFile;
        LogLevel    logLevel = LogLevel::kInfo;
};

auto get() -> Args&;
auto print_usage(const char* prog) -> void;
auto parseArguments(int argc, char* argv[]) -> bool;

} // namespace InputArgs

#endif // INPUTARGUMENTS_HPP
