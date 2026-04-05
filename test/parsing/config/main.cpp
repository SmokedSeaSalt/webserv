#include "parsing.hpp"
#include "configParsing.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../incl/doctest.h"

TEST_CASE("Test single ServerBlock (test1.conf)") {
    auto configResult = parseConfigFile("test1.conf");
    REQUIRE(configResult.has_value());

    const Config& config = configResult.value();
    REQUIRE(config.serverBlocks.size() == 1);

    const ServerBlock& serverBlock = config.serverBlocks[0];

    SUBCASE("listen directive")
    {
        CHECK(serverBlock.ip == "127.0.0.1");
        CHECK(serverBlock.port == 8080);
    }

    SUBCASE("client_max_body_size directive")
    {
        CHECK(serverBlock.maxBodySize == 10000);
    }

    SUBCASE("error_page directives")
    {
        REQUIRE(serverBlock.defaultErrorPages.count(404) == 1);
        REQUIRE(serverBlock.defaultErrorPages.count(500) == 1);
        CHECK(serverBlock.defaultErrorPages.at(404) == "/errors/404.html");
        CHECK(serverBlock.defaultErrorPages.at(500) == "/errors/500.html");

        REQUIRE(serverBlock.defaultErrorPages.count(400) != 1);
    }
}
