#include "inputArguments.hpp"
#include <iostream>
#include <string>
#include <unistd.h>

auto parseArguments() -> bool
{
}

static void print_usage(const char* prog)
{
    std::cerr << "Usage: " << prog << " <CONFIG_FILE>\n"
              << "  -p <path> Global relative path\n"
              << "  -l <file> Output logfile instead of stdout\n"
              << "  -d <n>    Debug log level. 0: Nothing, 1: Info, 2: Verbose, 3: Errors, 4: Debug\n"
              << "  -h        Print this message\n";
}

enum class LogLevel
{
    kSilent  = 0,
    kInfo    = 1,
    kVerbose = 2,
    kErrors  = 3,
    kDebug   = 4,
};

int main(int argc, char* argv[])
{
    std::string relativePath;
    std::string logFile;
    LogLevel    logLevel = LogLevel::kInfo;

    int opt;
    while ((opt = getopt(argc, argv, "p:l:d:h")) != -1)
    {
        switch (opt)
        {
        case 'p':
            relativePath = optarg;
            break;
        case 'l':
            logFile = optarg;
            break;
        case 'd':
            int ret;
            try
            {
                ret = std::stoi(optarg);
            }
            catch (const std::exception& e)
            {
                print_usage(argv[0]);
                return 1;
            }
            if (ret < 0 || ret > 4)
            {
                print_usage(argv[0]);
                return 1;
            }
            logLevel = static_cast<LogLevel>(ret);

            break;
        case 'h':
            print_usage(argv[0]);
            return 1;
        case '?':
            print_usage(argv[0]);
            return 1;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    for (int i = optind; i < argc; ++i)
    {
        std::cout << "positional arg: " << argv[i] << "\n";
    }
}