#include "configParsing.hpp"
#include "parsing.hpp"
#include <filesystem>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../incl/doctest.h"

TEST_CASE("Test single ServerBlock (test1.conf)")
{
    auto configResult = parseConfigFile("test_files/test1.conf");
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

    SUBCASE("locations")
    {
        REQUIRE(serverBlock.locations.size() == 5);

        SUBCASE("location0 root slash")
        {
            const Location& loc = serverBlock.locations[0];
            CHECK(loc.pathPrefix == "/");
            CHECK(loc.acceptedMethods.getAllowed == true);
            CHECK(loc.acceptedMethods.headAllowed == true);
            CHECK(loc.acceptedMethods.postAllowed == false);
            CHECK(loc.acceptedMethods.deleteAllowed == false);

            CHECK(loc.defaultFile == "index.html");
            CHECK(loc.directoryListing == false);

            CHECK(loc.root.empty());
            CHECK(loc.redirectCode == 0);
            CHECK(loc.redirectLocation.empty());
            CHECK(loc.uploadsAllowed == false);
            CHECK(loc.uploadLocation.empty());
            CHECK(loc.cgiPaths.empty());
        }

        SUBCASE("location1 images")
        {
            const Location& loc = serverBlock.locations[1];
            CHECK(loc.pathPrefix == "/images");
            CHECK(loc.acceptedMethods.getAllowed == true);
            CHECK(loc.acceptedMethods.headAllowed == false);
            CHECK(loc.acceptedMethods.postAllowed == false);
            CHECK(loc.acceptedMethods.deleteAllowed == false);

            CHECK(loc.root == "/var/www/images");
            CHECK(loc.defaultFile == "index.html");
            CHECK(loc.directoryListing == true);

            CHECK(loc.redirectCode == 0);
            CHECK(loc.redirectLocation.empty());
            CHECK(loc.uploadsAllowed == false);
            CHECK(loc.uploadLocation.empty());
            CHECK(loc.cgiPaths.empty());
        }

        SUBCASE("location2 upload")
        {
            const Location& loc = serverBlock.locations[2];
            CHECK(loc.pathPrefix == "/upload");
            CHECK(loc.acceptedMethods.getAllowed == false);
            CHECK(loc.acceptedMethods.headAllowed == false);
            CHECK(loc.acceptedMethods.postAllowed == true);
            CHECK(loc.acceptedMethods.deleteAllowed == false);

            CHECK(loc.uploadsAllowed == true);
            CHECK(loc.uploadLocation == "/tmp/uploads");

            CHECK(loc.root.empty());
            CHECK(loc.defaultFile.empty());
            CHECK(loc.directoryListing == false);
            CHECK(loc.redirectCode == 0);
            CHECK(loc.redirectLocation.empty());
            CHECK(loc.cgiPaths.empty());
        }

        SUBCASE("location3 redirect")
        {
            const Location& loc = serverBlock.locations[3];
            CHECK(loc.pathPrefix == "/old-page");
            CHECK(loc.redirectCode == 301);
            CHECK(loc.redirectLocation == "/new-page");

            CHECK(loc.acceptedMethods.getAllowed == false);
            CHECK(loc.acceptedMethods.headAllowed == false);
            CHECK(loc.acceptedMethods.postAllowed == false);
            CHECK(loc.acceptedMethods.deleteAllowed == false);

            CHECK(loc.root.empty());
            CHECK(loc.defaultFile.empty());
            CHECK(loc.directoryListing == false);
            CHECK(loc.uploadsAllowed == false);
            CHECK(loc.uploadLocation.empty());
            CHECK(loc.cgiPaths.empty());
        }

        SUBCASE("location4 scripts cgi")
        {
            const Location& loc = serverBlock.locations[4];
            CHECK(loc.pathPrefix == "/scripts");
            CHECK(loc.acceptedMethods.getAllowed == true);
            CHECK(loc.acceptedMethods.headAllowed == false);
            CHECK(loc.acceptedMethods.postAllowed == true);
            CHECK(loc.acceptedMethods.deleteAllowed == false);

            CHECK(loc.root == "/var/www/cgi");

            REQUIRE(loc.cgiPaths.count(".php") == 1);
            REQUIRE(loc.cgiPaths.count(".py") == 1);
            CHECK(loc.cgiPaths.at(".php") == "/usr/bin/php-cgi");
            CHECK(loc.cgiPaths.at(".py") == "/usr/bin/python3");

            CHECK(loc.defaultFile.empty());
            CHECK(loc.directoryListing == false);
            CHECK(loc.redirectCode == 0);
            CHECK(loc.redirectLocation.empty());
            CHECK(loc.uploadsAllowed == false);
            CHECK(loc.uploadLocation.empty());
        }
    }
}

TEST_CASE("Test multiple ServerBlocks (test2.conf)")
{
    auto configResult = parseConfigFile("test_files/test2.conf");
    REQUIRE_MESSAGE(configResult.has_value(),
                    "parseConfigFile(\"test2.conf\") failed: " << configResult.error());

    const Config& config = configResult.value();
    REQUIRE(config.serverBlocks.size() == 2);

    SUBCASE("server block 0")
    {
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
        }

        SUBCASE("locations")
        {
            REQUIRE(serverBlock.locations.size() == 5);

            SUBCASE("location0 root slash")
            {
                const Location& loc = serverBlock.locations[0];
                CHECK(loc.pathPrefix == "/");
                CHECK(loc.acceptedMethods.getAllowed == true);
                CHECK(loc.acceptedMethods.headAllowed == true);
                CHECK(loc.acceptedMethods.postAllowed == false);
                CHECK(loc.acceptedMethods.deleteAllowed == false);

                CHECK(loc.defaultFile == "index.html");
                CHECK(loc.directoryListing == false);

                CHECK(loc.root.empty());
                CHECK(loc.redirectCode == 0);
                CHECK(loc.redirectLocation.empty());
                CHECK(loc.uploadsAllowed == false);
                CHECK(loc.uploadLocation.empty());
                CHECK(loc.cgiPaths.empty());
            }

            SUBCASE("location1 images")
            {
                const Location& loc = serverBlock.locations[1];
                CHECK(loc.pathPrefix == "/images");
                CHECK(loc.acceptedMethods.getAllowed == true);
                CHECK(loc.acceptedMethods.headAllowed == false);
                CHECK(loc.acceptedMethods.postAllowed == false);
                CHECK(loc.acceptedMethods.deleteAllowed == false);

                CHECK(loc.root == "/var/www/images");
                CHECK(loc.defaultFile == "index.html");
                CHECK(loc.directoryListing == true);

                CHECK(loc.redirectCode == 0);
                CHECK(loc.redirectLocation.empty());
                CHECK(loc.uploadsAllowed == false);
                CHECK(loc.uploadLocation.empty());

                REQUIRE(loc.cgiPaths.count(".php") == 1);
                CHECK(loc.cgiPaths.at(".php") == "/usr/bin/php-cgi");
            }

            SUBCASE("location2 upload")
            {
                const Location& loc = serverBlock.locations[2];
                CHECK(loc.pathPrefix == "/upload");
                CHECK(loc.acceptedMethods.getAllowed == false);
                CHECK(loc.acceptedMethods.headAllowed == false);
                CHECK(loc.acceptedMethods.postAllowed == true);
                CHECK(loc.acceptedMethods.deleteAllowed == true);

                CHECK(loc.uploadsAllowed == true);
                CHECK(loc.uploadLocation == "/tmp/uploads");

                CHECK(loc.root.empty());
                CHECK(loc.defaultFile.empty());
                CHECK(loc.directoryListing == false);
                CHECK(loc.redirectCode == 0);
                CHECK(loc.redirectLocation.empty());
                CHECK(loc.cgiPaths.empty());
            }

            SUBCASE("location3 redirect")
            {
                const Location& loc = serverBlock.locations[3];
                CHECK(loc.pathPrefix == "/old-page");
                CHECK(loc.redirectCode == 301);
                CHECK(loc.redirectLocation == "/new-page");

                CHECK(loc.acceptedMethods.getAllowed == false);
                CHECK(loc.acceptedMethods.headAllowed == false);
                CHECK(loc.acceptedMethods.postAllowed == false);
                CHECK(loc.acceptedMethods.deleteAllowed == false);

                CHECK(loc.root.empty());
                CHECK(loc.defaultFile.empty());
                CHECK(loc.directoryListing == false);
                CHECK(loc.uploadsAllowed == false);
                CHECK(loc.uploadLocation.empty());
                CHECK(loc.cgiPaths.empty());
            }

            SUBCASE("location4 scripts cgi")
            {
                const Location& loc = serverBlock.locations[4];
                CHECK(loc.pathPrefix == "/scripts");
                CHECK(loc.acceptedMethods.getAllowed == true);
                CHECK(loc.acceptedMethods.headAllowed == false);
                CHECK(loc.acceptedMethods.postAllowed == true);
                CHECK(loc.acceptedMethods.deleteAllowed == false);

                CHECK(loc.root == "/var/www/cgi");

                REQUIRE(loc.cgiPaths.count(".php") == 1);
                REQUIRE(loc.cgiPaths.count(".py") == 1);
                CHECK(loc.cgiPaths.at(".php") == "/usr/bin/php-cgi");
                CHECK(loc.cgiPaths.at(".py") == "/usr/bin/python3");

                CHECK(loc.defaultFile.empty());
                CHECK(loc.directoryListing == false);
                CHECK(loc.redirectCode == 0);
                CHECK(loc.redirectLocation.empty());
                CHECK(loc.uploadsAllowed == false);
                CHECK(loc.uploadLocation.empty());
            }
        }
    }

    SUBCASE("server block 1")
    {
        const ServerBlock& serverBlock = config.serverBlocks[1];

        SUBCASE("listen directive")
        {
            CHECK(serverBlock.ip == "0.0.0.0");
            CHECK(serverBlock.port == 9090);
        }

        SUBCASE("client_max_body_size directive")
        {
            CHECK(serverBlock.maxBodySize == 5000);
        }

        SUBCASE("error_page directives")
        {
            REQUIRE(serverBlock.defaultErrorPages.count(404) == 1);
            CHECK(serverBlock.defaultErrorPages.at(404) == "/errors/404.html");
            CHECK(serverBlock.defaultErrorPages.count(500) == 0);
        }

        SUBCASE("locations")
        {
            REQUIRE(serverBlock.locations.size() == 4);

            SUBCASE("location0 root slash")
            {
                const Location& loc = serverBlock.locations[0];
                CHECK(loc.pathPrefix == "/");
                CHECK(loc.acceptedMethods.getAllowed == true);
                CHECK(loc.acceptedMethods.headAllowed == false);
                CHECK(loc.acceptedMethods.postAllowed == true);
                CHECK(loc.acceptedMethods.deleteAllowed == true);

                CHECK(loc.defaultFile == "index.html");
                CHECK(loc.directoryListing == false);

                CHECK(loc.root.empty());
                CHECK(loc.redirectCode == 0);
                CHECK(loc.redirectLocation.empty());
                CHECK(loc.uploadsAllowed == false);
                CHECK(loc.uploadLocation.empty());
                CHECK(loc.cgiPaths.empty());
            }

            SUBCASE("location1 api")
            {
                const Location& loc = serverBlock.locations[1];
                CHECK(loc.pathPrefix == "/api");
                CHECK(loc.acceptedMethods.getAllowed == true);
                CHECK(loc.acceptedMethods.headAllowed == false);
                CHECK(loc.acceptedMethods.postAllowed == true);
                CHECK(loc.acceptedMethods.deleteAllowed == true);

                CHECK(loc.root == "/var/www/api");
                CHECK(loc.directoryListing == false);

                REQUIRE(loc.cgiPaths.count(".py") == 1);
                CHECK(loc.cgiPaths.at(".py") == "/usr/bin/python3");

                CHECK(loc.defaultFile.empty());
                CHECK(loc.redirectCode == 0);
                CHECK(loc.redirectLocation.empty());
                CHECK(loc.uploadsAllowed == false);
                CHECK(loc.uploadLocation.empty());
            }

            SUBCASE("location2 files")
            {
                const Location& loc = serverBlock.locations[2];
                CHECK(loc.pathPrefix == "/files");
                CHECK(loc.acceptedMethods.getAllowed == true);
                CHECK(loc.acceptedMethods.headAllowed == false);
                CHECK(loc.acceptedMethods.postAllowed == false);
                CHECK(loc.acceptedMethods.deleteAllowed == false);

                CHECK(loc.root == "/var/www/files");
                CHECK(loc.directoryListing == true);

                CHECK(loc.defaultFile.empty());
                CHECK(loc.redirectCode == 0);
                CHECK(loc.redirectLocation.empty());
                CHECK(loc.uploadsAllowed == false);
                CHECK(loc.uploadLocation.empty());
                CHECK(loc.cgiPaths.empty());
            }

            SUBCASE("location3 incoming upload")
            {
                const Location& loc = serverBlock.locations[3];
                CHECK(loc.pathPrefix == "/incoming");
                CHECK(loc.acceptedMethods.getAllowed == false);
                CHECK(loc.acceptedMethods.headAllowed == false);
                CHECK(loc.acceptedMethods.postAllowed == true);
                CHECK(loc.acceptedMethods.deleteAllowed == false);

                CHECK(loc.uploadsAllowed == true);
                CHECK(loc.uploadLocation == "/tmp/incoming");

                CHECK(loc.root.empty());
                CHECK(loc.defaultFile.empty());
                CHECK(loc.directoryListing == false);
                CHECK(loc.redirectCode == 0);
                CHECK(loc.redirectLocation.empty());
                CHECK(loc.cgiPaths.empty());
            }
        }
    }
}

TEST_CASE("Test parsing errors")
{
    const std::vector<std::string> invalidConfigs = {
        "test_files/invalidMaxBodySize.conf", "test_files/invalidMaxBodySizeCount.conf",
        "test_files/invalidListen.conf",      "test_files/invalidListenCount.conf",
        "test_files/invalidErrorPage.conf",   "test_files/invalidLocation.conf",
        "test_files/invalidRedirect.conf",    "test_files/invalidCGI.conf",
        "test_files/invalidAutoIndex.conf",   "test_files/invalidSemicolon.conf",
    };

    for (size_t i = 0; i < invalidConfigs.size(); ++i)
    {
        const std::string& path = invalidConfigs[i];

        SUBCASE(path.c_str())
        {
            auto configResult = parseConfigFile(path);
            CHECK(configResult.has_value() == false);
            CHECK(configResult.error().empty() == false);
            CHECK_MESSAGE(
                configResult.error() != "Config file could not be opened",
                ("File could not be opened. Check permisions or existence of file: " + path));
        }
    }
}

TEST_CASE("Test file opening errors")
{
    SUBCASE("Non-existent file")
    {
        auto configResult = parseConfigFile("blahblah123.conf");
        CHECK(configResult.has_value() == false);
        CHECK(configResult.error() == "Config file could not be opened");
    }

    SUBCASE("File with no read permissions")
    {
        // const std::string filepath = "test_files/noPermissions.conf";

        // // Remove all permissions
        // std::filesystem::permissions(filepath, std::filesystem::perms::none);

        // // Test that parse fails
        // auto configResult = parseConfigFile(filepath);
        // CHECK(configResult.has_value() == false);
        // CHECK(configResult.error() == "Config file could not be opened");
    }
}