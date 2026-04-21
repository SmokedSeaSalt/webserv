#ifndef INPUTARGUMENTS_HPP
#define INPUTARGUMENTS_HPP

#include "logging.hpp"
#include <string>

namespace InputArgs
{

struct Args
{
        std::string relativePath;
        std::string logFile;
        std::string configFile;
        LogLevel    logLevel = LogLevel::kInfo;
};

inline Args args;

auto print_usage(const char* prog) -> void;
auto parseArguments(int argc, char* argv[]) -> bool;

} // namespace InputArgs

#endif // INPUTARGUMENTS_HPP
