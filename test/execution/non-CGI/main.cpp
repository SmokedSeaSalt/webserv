#include "Execution.hpp"
#include "HTTPResponse.hpp"
#include "configParsing.hpp"
#include "executionHelpers.hpp"
#include <filesystem>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../incl/doctest.h"

//////////////////
// helpers ///////
/////////////////
static auto loadBinaryFile(const std::filesystem::path& path) -> std::string
{
    std::ifstream in(path, std::ios::binary);
    REQUIRE(in.is_open());
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

auto checkPacket(std::string packet, std::string expectedBody) -> bool
{
    std::istringstream stream(packet);
    std::string        line;
    bool               foundStatus, foundServer, foundDate, foundBody = false;

    int lineNum = 0;
    while (std::getline(stream, line, '\n'))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (lineNum == 0 && line == "HTTP/1.1 200 OK")
            foundStatus = true;
        if (line.find("server: webserv") != std::string::npos)
            foundServer = true;
        if (line.find("date: ") != 0)
            foundDate = true;
        if (line == expectedBody)
            foundBody = true;
        lineNum++;
    }

    CHECK(foundStatus);
    CHECK(foundServer);
    CHECK(foundDate);
    CHECK(foundBody);
    return foundStatus && foundServer && foundDate && foundBody;
}

//////////////////
// readFile tests/
//////////////////
TEST_CASE("Test readFile html (200)")
{
    std::filesystem::path filePath       = std::filesystem::current_path() / "assets" / "helloWorld.html";
    auto                  readFileResult = readFile(filePath.string());
    REQUIRE(readFileResult.has_value());
    CHECK(readFileResult.value() == "<p>Hello world</p>");
}

TEST_CASE("Test readFile png (200)")
{
    int                   len            = 23997;
    std::filesystem::path filePath       = std::filesystem::current_path() / "assets/example.png";
    auto                  readFileResult = readFile(filePath.string());
    REQUIRE(readFileResult.has_value());
    CHECK(readFileResult.value().length() == len);
    CHECK(readFileResult.value() == loadBinaryFile(filePath));
}

TEST_CASE("Test readFile in forbidden folder (403)")
{
    std::filesystem::path filePath       = std::filesystem::current_path() / "assets/noPermissions/other.html";
    auto                  readFileResult = readFile(filePath.string());
    REQUIRE(!readFileResult.has_value());
    CHECK(readFileResult.error() == ResponseStatusCode::kForbidden);
}

TEST_CASE("Test readFile non existent file (404)")
{
    std::filesystem::path filePath       = std::filesystem::current_path() / "assets/abc123nonExistent.html";
    auto                  readFileResult = readFile(filePath.string());
    REQUIRE(!readFileResult.has_value());
    CHECK(readFileResult.error() == ResponseStatusCode::kNotFound);
}

/////////////////
// execute tests/
/////////////////

TEST_CASE("Test full path get html file")
{
    auto configResult = parseConfigFile("config.conf");
    REQUIRE(configResult.has_value());
    Config      config = configResult.value();
    Execution   execution(config);
    std::string path = std::filesystem::current_path() / "assets/helloWorld.html";

    HTTPMessage httpMessage;
    httpMessage.method            = "GET";
    httpMessage.requestTarget     = path;
    httpMessage.protocol          = "HTTP/1.1";
    httpMessage.headers["host"]   = {"localhost:8080"};
    httpMessage.headers["accept"] = {"text/html"};
    httpMessage.body              = "";

    CHECK(httpMessage.method == "GET");
    CHECK(httpMessage.requestTarget == path);
    CHECK(httpMessage.protocol == "HTTP/1.1");

    HTTPResponse response = execution.execute(httpMessage);
    checkPacket(response.createPacket(), "<p>Hello world</p>");
    // CHECK(repsonse. == "");
}

TEST_CASE("Test full path get png file test")
{
    auto configResult = parseConfigFile("config.conf");
    REQUIRE(configResult.has_value());
    Config      config = configResult.value();
    Execution   execution(config);
    std::string path         = std::filesystem::current_path() / "assets/example.png";
    std::string expectedBody = loadBinaryFile(path);

    HTTPMessage httpMessage;
    httpMessage.method            = "GET";
    httpMessage.requestTarget     = path;
    httpMessage.protocol          = "HTTP/1.1";
    httpMessage.headers["host"]   = {"localhost:8080"};
    httpMessage.headers["accept"] = {"text/html"};
    httpMessage.body              = "";

    CHECK(httpMessage.method == "GET");
    CHECK(httpMessage.requestTarget == path);
    CHECK(httpMessage.protocol == "HTTP/1.1");

    HTTPResponse response = execution.execute(httpMessage);
    std::string  packet   = response.createPacket();

    CHECK(packet.rfind("HTTP/1.1 200 OK\r\n", 0) == 0);
    // TODO: also check if header content-type is set correctly
    const std::string sep = "\r\n\r\n";

    size_t bodyPos = packet.find(sep);
    REQUIRE(bodyPos != std::string::npos);

    std::string body = packet.substr(bodyPos + sep.size());

    CHECK(body.size() == expectedBody.size());
    CHECK(body == expectedBody);
}

// TEST_CASE("Hardcoded full path get html file in forbidden folder test")
// {
//     auto configResult = parseConfigFile("config.conf");
//     REQUIRE(configResult.has_value());
//     Config    config = configResult.value();
//     Execution execution(config);

//     HTTPMessage httpMessage;
//     httpMessage.method            = "GET";
//     httpMessage.requestTarget     = "/home/egrisel/Repos/rank05/webserv_personal/test/execution/non-CGI/assets/noPermissions/other.html";
//     httpMessage.protocol          = "HTTP/1.1";
//     httpMessage.headers["host"]   = {"localhost:8080"};
//     httpMessage.headers["accept"] = {"text/html"};
//     httpMessage.body              = "";

//     CHECK(httpMessage.method == "GET");
//     CHECK(httpMessage.requestTarget == "/home/egrisel/Repos/rank05/webserv_personal/test/execution/non-CGI/assets/noPermissions/other.html");
//     CHECK(httpMessage.protocol == "HTTP/1.1");

//     HTTPResponse response = execution.execute(httpMessage);
//     CHECK(response == ResponseStatusCode::kForbidden);
//     // CHECK(repsonse. == "");
// }
