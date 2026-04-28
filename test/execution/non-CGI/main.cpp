#include "Execution.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRules.hpp"
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

auto checkPacket(std::string packet, std::string expectedContentType, std::string expectedBody, ResponseStatusCode code = ResponseStatusCode::kOK) -> bool
{
    std::istringstream stream(packet);
    std::string        line;
    bool               foundStatus = false, foundServer = false, foundDate = false, foundContentType = false, foundBody = false;
    int                lineNum = 0;
    while (std::getline(stream, line, '\n'))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (lineNum == 0 && line == ("HTTP/1.1 " + HTTPRules::statusCodeToString(code)))
            foundStatus = true;
        if (line.find("server: webserv") != std::string::npos)
            foundServer = true;
        if (line.find("date: ") == 0)
            foundDate = true;
        if (line.find("content-type: " + expectedContentType) == 0)
            foundContentType = true;
        if (line == expectedBody)
            foundBody = true;
        lineNum++;
    }

    CHECK(foundStatus);
    CHECK(foundServer);
    CHECK(foundDate);
    CHECK(foundContentType);
    CHECK(foundBody);
    return foundStatus && foundServer && foundDate && foundBody;
}

//////////////////
// content type tests
//////////////////
TEST_CASE("test content types")
{
    // Images
    CHECK(fileExtensionToContentType("assets/img.png") == "image/png");
    CHECK(fileExtensionToContentType("assets/img.jpg") == "image/jpeg");
    CHECK(fileExtensionToContentType("assets/img.jpeg") == "image/jpeg");
    CHECK(fileExtensionToContentType("assets/img.gif") == "image/gif");
    CHECK(fileExtensionToContentType("assets/img.webp") == "image/webp");
    CHECK(fileExtensionToContentType("assets/img.svg") == "image/svg+xml");
    CHECK(fileExtensionToContentType("assets/img.ico") == "image/x-icon");
    // Web
    CHECK(fileExtensionToContentType("assets/index.html") == "text/html; charset=utf-8");
    CHECK(fileExtensionToContentType("assets/style.css") == "text/css");
    CHECK(fileExtensionToContentType("assets/app.js") == "application/javascript");
    CHECK(fileExtensionToContentType("assets/data.json") == "application/json");
    // Misc
    CHECK(fileExtensionToContentType("assets/file.pdf") == "application/pdf");
    CHECK(fileExtensionToContentType("assets/file.txt") == "text/plain; charset=utf-8");
    CHECK(fileExtensionToContentType("assets/file.xml") == "application/xml");
    // Extras
    CHECK(fileExtensionToContentType("assets/file.com.xml") == "application/xml");
    CHECK(fileExtensionToContentType("ass.ets/img.png") == "image/png");
    CHECK(fileExtensionToContentType("assets/anotherFolder/file") == "application/octet-stream");
    CHECK(fileExtensionToContentType("file.mp4") == "application/octet-stream");
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
    std::string fullPath = std::filesystem::current_path() / "assets/noPermissions/other.html";
    std::system(("chmod 000 " + fullPath).c_str());
    std::filesystem::path filePath       = fullPath;
    auto                  readFileResult = readFile(filePath.string());
    REQUIRE(!readFileResult.has_value());
    CHECK(readFileResult.error() == ResponseStatusCode::kForbidden);
    std::system(("chmod 777 " + fullPath).c_str());
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
    Config::config    = {};
    auto configResult = Config::parseConfigFile("config.conf");
    REQUIRE(configResult.has_value());
    Execution   execution{};
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
    checkPacket(response.createPacket(), "text/html; charset=utf-8", "<p>Hello world</p>");
    // CHECK(repsonse. == "");
}

TEST_CASE("Test full path get png file test")
{
    Config::config    = {};
    auto configResult = Config::parseConfigFile("config.conf");
    REQUIRE(configResult.has_value());
    Execution   execution{};
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
    CHECK(packet.find("content-type: image/png\r\n", 0) != std::string::npos);

    // TODO: also check if header content-type is set correctly
    const std::string sep = "\r\n\r\n";

    size_t bodyPos = packet.find(sep);
    REQUIRE(bodyPos != std::string::npos);

    std::string body = packet.substr(bodyPos + sep.size());

    CHECK(body.size() == expectedBody.size());
    CHECK(body == expectedBody);
}

TEST_CASE("Test HEAD")
{
    Config::config    = {};
    auto configResult = Config::parseConfigFile("config.conf");
    REQUIRE(configResult.has_value());
    Execution   execution{};
    std::string path = std::filesystem::current_path() / "assets/helloWorld.html";

    HTTPMessage httpMessage;
    httpMessage.method            = "HEAD";
    httpMessage.requestTarget     = path;
    httpMessage.protocol          = "HTTP/1.1";
    httpMessage.headers["host"]   = {"localhost:8080"};
    httpMessage.headers["accept"] = {"text/html"};
    httpMessage.body              = "";

    CHECK(httpMessage.method == "HEAD");
    CHECK(httpMessage.requestTarget == path);
    CHECK(httpMessage.protocol == "HTTP/1.1");

    HTTPResponse response = execution.execute(httpMessage);
    checkPacket(response.createPacket(), "text/html; charset=utf-8", "");
    // CHECK(repsonse. == "");
}

TEST_CASE("Test POST and then GET the file with manual file delete")
{
    Config::config    = {};
    auto configResult = Config::parseConfigFile("config.conf");
    REQUIRE(configResult.has_value());
    Execution   execution{};
    std::string path         = std::filesystem::current_path() / "assets/newFile.html";
    std::string fileContents = "<h>This file has been posted</h>";

    std::system(("rm -rf " + path).c_str());

    HTTPMessage httpPostMessage;
    httpPostMessage.method          = "POST";
    httpPostMessage.requestTarget   = path;
    httpPostMessage.protocol        = "HTTP/1.1";
    httpPostMessage.headers["host"] = {"localhost:8080"};
    httpPostMessage.body            = fileContents;

    CHECK(httpPostMessage.method == "POST");
    CHECK(httpPostMessage.requestTarget == path);
    CHECK(httpPostMessage.protocol == "HTTP/1.1");

    HTTPResponse postResponse1 = execution.execute(httpPostMessage);
    CHECK(postResponse1.createPacket().find("201") != std::string::npos);

    HTTPResponse postResponse2 = execution.execute(httpPostMessage);
    CHECK(postResponse2.createPacket().find("409") != std::string::npos);

    HTTPMessage httpGetMessage;
    httpGetMessage.method            = "GET";
    httpGetMessage.requestTarget     = path;
    httpGetMessage.protocol          = "HTTP/1.1";
    httpGetMessage.headers["host"]   = {"localhost:8080"};
    httpGetMessage.headers["accept"] = {"text/html"};
    httpGetMessage.body              = "";

    HTTPResponse getResponse = execution.execute(httpGetMessage);
    checkPacket(getResponse.createPacket(), "text/html; charset=utf-8", fileContents);

    // CHECK(repsonse. == "");
    std::system(("rm -rf " + path).c_str());
}

TEST_CASE("Test POST and then GET and then DELETE")
{
    Config::config    = {};
    auto configResult = Config::parseConfigFile("config.conf");
    REQUIRE(configResult.has_value());
    Execution   execution{};
    std::string path         = std::filesystem::current_path() / "assets/newFile.html";
    std::string fileContents = "<h>This file has been posted</h>";

    std::system(("rm -rf " + path).c_str());

    // POST the file
    HTTPMessage httpPostMessage;
    httpPostMessage.method          = "POST";
    httpPostMessage.requestTarget   = path;
    httpPostMessage.protocol        = "HTTP/1.1";
    httpPostMessage.headers["host"] = {"localhost:8080"};
    httpPostMessage.body            = fileContents;
    HTTPResponse postResponse       = execution.execute(httpPostMessage);
    CHECK(postResponse.createPacket().find("201") != std::string::npos);

    // GET the file
    HTTPMessage httpGetMessage;
    httpGetMessage.method            = "GET";
    httpGetMessage.requestTarget     = path;
    httpGetMessage.protocol          = "HTTP/1.1";
    httpGetMessage.headers["host"]   = {"localhost:8080"};
    httpGetMessage.headers["accept"] = {"text/html"};
    httpGetMessage.body              = "";
    HTTPResponse getResponse1        = execution.execute(httpGetMessage);
    checkPacket(getResponse1.createPacket(), "text/html; charset=utf-8", fileContents);

    // DELETE the file
    HTTPMessage httpDeleteMessage;
    httpDeleteMessage.method          = "DELETE";
    httpDeleteMessage.requestTarget   = path;
    httpDeleteMessage.protocol        = "HTTP/1.1";
    httpDeleteMessage.headers["host"] = {"localhost:8080"};
    httpDeleteMessage.body            = "";
    HTTPResponse deleteResponse       = execution.execute(httpDeleteMessage);
    CHECK(deleteResponse.createPacket().find("204") != std::string::npos);

    // GET the already deleted file (should fail)
    HTTPResponse getResponse2 = execution.execute(httpGetMessage);
    CHECK(getResponse2.createPacket().find("404") != std::string::npos);
}
