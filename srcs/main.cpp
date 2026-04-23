#include "InputArgs.hpp"
#include "Server.hpp"
#include "configParsing.hpp"
#include "logging.hpp"

int main(int argc, char** argv)
{
    if (!InputArgs::parseArguments(argc, argv))
        return 1;
    Logging::init(InputArgs::args.logFile.c_str(), InputArgs::args.logLevel);
    auto config = parseConfigFile(InputArgs::args.configFile);
    if (!config.has_value())
        return 1; // todo error handling
    Server server = Server(config.value());
    server.setup();
    server.connection_loop();
}