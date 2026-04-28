#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../incl/doctest.h"
#include "Server.hpp"
#include "configParsing.hpp"
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <thread>

void mockParseConfig(std::string ip, int port)
{
    Config::config = {};
    Config::ServerBlock sb{};
    sb.ip                = ip;
    sb.port              = port;
    sb.defaultErrorPages = {{404, "/errors/404.html"}, {500, "/errors/500.html"}};
    sb.maxBodySize       = 10000;
    sb.locations         = {}; // leave empty for connection tests

    Config::config.serverBlocks.push_back(sb);
}

TEST_CASE("Server receives and prints request")
{
    const char* portEnv = std::getenv("PORT");
    int         port    = std::stoi(portEnv ? portEnv : "4242");
    mockParseConfig("", port);
    Server server{};

    // Setup server
    auto setupRes = server.setup();
    CHECK(setupRes.has_value());

    // Run server in background thread
    std::thread serverThread([&server]()
                             { server.connection_loop(); });
    serverThread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // TODO: Later verify response here
    // CHECK(response == "HTTP/1.1 200 OK\r\n");
}
