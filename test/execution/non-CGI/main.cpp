#include "Execution.hpp"
#include "configParsing.hpp"
#include "executionHelpers.hpp"
#include "HTTPResponse.hpp"
#include <filesystem>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../incl/doctest.h"

TEST_CASE("Test readFile")
{
    std::filesystem::path filePath       = std::filesystem::current_path() / "assets" / "helloWorld.html";
    auto                  readFileResult = readFile(filePath.string());
    REQUIRE(readFileResult.has_value());
    CHECK(readFileResult.value() == "<p>Hello world</p>");
}

auto checkPacket(std::string packet) -> bool
{
    std::istringstream stream(packet);
    std::string line;
    bool foundStatus, foundServer, foundDate, foundBody = false;

    int lineNum = 0;
    while (std::getline(stream, line, '\n'))
    {
        if (lineNum == 3)
        {
            lineNum++;
            continue;
        }
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (lineNum == 0 && line == "HTTP/1.1 200 OK")
            foundStatus = true;
        if (lineNum == 1 && line.find("server: webserv") != std::string::npos)
            foundServer = true;
        if (lineNum == 2 && line.find("date: ") != 0)
            foundDate = true;
        if (lineNum == 4 && line == "<p>Hello world</p>")
            foundBody = true;
        lineNum++;
    }
    return foundStatus && foundServer && foundDate && foundBody;
}

// temp test case
TEST_CASE("Hardcoded full path get test")
{
    auto configResult = parseConfigFile("config.conf");
    REQUIRE(configResult.has_value());
    Config    config = configResult.value();
    Execution execution(config);

    HTTPMessage httpMessage;
    httpMessage.method            = "GET";
    httpMessage.requestTarget     = "/home/egrisel/Repos/rank05/webserv_personal/test/execution/non-CGI/assets/helloWorld.html";
    httpMessage.protocol          = "HTTP/1.1";
    httpMessage.headers["host"]   = {"localhost:8080"};
    httpMessage.headers["accept"] = {"text/html"};
    httpMessage.body              = "";

    CHECK(httpMessage.method == "GET");
    CHECK(httpMessage.requestTarget == "/home/egrisel/Repos/rank05/webserv_personal/test/execution/non-CGI/assets/helloWorld.html");
    CHECK(httpMessage.protocol == "HTTP/1.1");

    HTTPResponse response = execution.execute(httpMessage);
    CHECK(checkPacket(response.createPacket()));
    // CHECK(repsonse. == "");
}