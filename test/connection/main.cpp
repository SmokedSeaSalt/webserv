#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../incl/doctest.h"

#include "../../incl/connection/Server.hpp"
#include <thread>
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>


Config  mockParseConfig(std::string port, int ip)
{
	Config config;
	config.serverBlocks.push_back(ServerBlock(port, ip));
	return config;
}

TEST_CASE("Server receives and prints request") {
    int port = std::stoi(std::getenv("PORT"));
    Config config = mockParseConfig("", port);
    Server server(config);
    
    // Setup server
    auto setupRes = server.setup();
    CHECK(setupRes.has_value());
    
    // Run server in background thread
    std::thread serverThread([&server]() {
        server.connection_loop();
    });
    serverThread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    

    
    // TODO: Later verify response here
    // CHECK(response == "HTTP/1.1 200 OK\r\n");
}
