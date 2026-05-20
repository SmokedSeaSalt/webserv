#include "InputArgs.hpp"
#include <iostream>
#include <unistd.h>

namespace InputArgs
{

auto print_usage(const char* prog) -> void
{
    std::cerr << "Usage: [flags] " << prog << " <CONFIG_FILE>\n"
              << "  -p <path>    Global relative path\n"
              << "  -l <file>    Output logfile instead of stdout\n"
              << "  -d <n>       Debug log level. 0: Nothing, 1: Info, 2: Verbose, 3: Errors, 4: Debug\n"
              << "  -t <seconds> client timeout\n"
              << "  -h           Print this message\n";
}

auto parseArguments(int argc, char* argv[]) -> bool
{
    optind = 0; // This fixes some edgecase withing the library.
    int opt;
    while ((opt = getopt(argc, argv, "p:l:d:t:h")) != -1)
    {
        switch (opt)
        {
        case 'p':
            args.relativePath = optarg;
            break;
        case 'l':
            args.logFile = optarg;
            break;
        case 'd':
        {
            int ret;
            try
            {
                ret = std::stoi(optarg);
            }
            catch (const std::exception& e)
            {
                print_usage(argv[0]);
                return false;
            }
            if (ret < 0 || ret > 4)
            {
                print_usage(argv[0]);
                return false;
            }
            args.logLevel = static_cast<LogLevel>(ret);
            break;
        }

        case 't':
        {
            int ret;
            try
            {
                ret = std::stoi(optarg);
            }
            catch (const std::exception& e)
            {
                print_usage(argv[0]);
                return false;
            }
            if (ret < 0)
            {
                print_usage(argv[0]);
                return false;
            }
            args.timeout = ret;
            break;
        }

        case 'h':
            print_usage(argv[0]);
            return false;
        case '?':
            print_usage(argv[0]);
            return false;
        default:
            print_usage(argv[0]);
            return false;
        }
    }

    // all flags should be parsed only config file should be left
    if (optind != (argc - 1))
    {
        print_usage(argv[0]);
        return false;
    }
    args.configFile = argv[optind];

    return true;
}

} // namespace InputArgs