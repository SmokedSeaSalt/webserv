#include "InputArgs.hpp"
#include "Server.hpp"
#include "configParsing.hpp"
#include "logging.hpp"

int main(int argc, char** argv)
{
    if (!InputArgs::parseArguments(argc, argv))
        return 1;
    Logging::init(InputArgs::args.logFile.c_str(), InputArgs::args.logLevel);
    auto ret = Config::parseConfigFile(InputArgs::args.configFile);
    if (!ret.has_value())
        return 1; // todo error handling
    Server server = Server();
    server.setup();
    server.connection_loop();
}