#include "InputArgs.hpp"
#include "Server.hpp"
#include "configParsing.hpp"
#include "logging.hpp"
#include "signals.hpp"

int main(int argc, char** argv)
{
    if (!InputArgs::parseArguments(argc, argv))
        return 1;
    Logging::init(InputArgs::args.logFile, InputArgs::args.logLevel);
    auto ret = Config::parseConfigFile(InputArgs::args.configFile);
    if (!ret.has_value())
    {
        std::cerr << ret.error() << "Config parsing error. Shutting down webserv" << std::endl;
        return 1; // todo error handling
    }
    if (Signals::initSignals() == -1)
    {
        LOG(LogLevel::kInfo, "initSignals failed. Shutting down.");
        return 1;
    }
    Server server = Server();
    server.setup();
    server.connection_loop();
}
