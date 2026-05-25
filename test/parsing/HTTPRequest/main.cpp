#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../incl/doctest.h"
#include "HTTPRequest.hpp"
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// Basic                                                                      //
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Basic test")
{
    HTTPRequest request{};
    std::string basic = "GET /index.html HTTP/1.1\r\n"
                        "Host: localhost:8080\r\n"
                        "User-Agent: curl/7.68.0\r\n"
                        "Accept: */*\r\n"
                        "\r\n";

    auto ret = request.newData(basic);
    REQUIRE_MESSAGE(ret.has_value(), "Error val: ", static_cast<int>(ret.error()));

    SUBCASE("First line")
    {
        CHECK(request.getMessage().method == "GET");
        CHECK(request.getMessage().requestTarget == "/index.html");
        CHECK(request.getMessage().protocol == "HTTP/1.1");
    }
    SUBCASE("Headers")
    {
        REQUIRE(request.getMessage().headers.contains("host"));
        CHECK(request.getMessage().headers["host"][0] == "localhost:8080");
        REQUIRE(request.getMessage().headers.contains("user-agent"));
        CHECK(request.getMessage().headers["user-agent"][0] == "curl/7.68.0");
        REQUIRE(request.getMessage().headers.contains("accept"));
        CHECK(request.getMessage().headers["accept"][0] == "*/*");
    }
    SUBCASE("Body")
    {
        CHECK(request.getMessage().body.empty());
    }
}

TEST_CASE("Basic test per byte")
{
    HTTPRequest request{};
    std::string basic = "GET /index.html HTTP/1.1\r\n"
                        "Host: localhost:8080\r\n"
                        "User-Agent: curl/7.68.0\r\n"
                        "Accept: */*\r\n"
                        "\r\n";

    size_t i = 0;
    while (i < basic.length())
    {
        auto ret = request.newData(basic.substr(i, 1));
        REQUIRE_MESSAGE(ret.has_value(), "Error val: ", static_cast<int>(ret.error()));
        i++;
    }

    SUBCASE("First line")
    {
        CHECK(request.getMessage().method == "GET");
        CHECK(request.getMessage().requestTarget == "/index.html");
        CHECK(request.getMessage().protocol == "HTTP/1.1");
    }
    SUBCASE("Headers")
    {
        REQUIRE(request.getMessage().headers.contains("host"));
        CHECK(request.getMessage().headers["host"][0] == "localhost:8080");
        REQUIRE(request.getMessage().headers.contains("user-agent"));
        CHECK(request.getMessage().headers["user-agent"][0] == "curl/7.68.0");
        REQUIRE(request.getMessage().headers.contains("accept"));
        CHECK(request.getMessage().headers["accept"][0] == "*/*");
    }
    SUBCASE("Body")
    {
        CHECK(request.getMessage().body.empty());
    }
}

////////////////////////////////////////////////////////////////////////////////
// Including body                                                             //
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Basic body test")
{
    HTTPRequest request{};
    std::string basic = "GET /index.html HTTP/1.1\r\n"
                        "Host: localhost:8080\r\n"
                        "User-Agent: curl/7.68.0\r\n"
                        "Content-Length: 8\r\n"
                        "Accept: */*\r\n"
                        "\r\n"
                        "abcdefgh";

    auto ret = request.newData(basic);
    REQUIRE_MESSAGE(ret.has_value(), "Error val: ", static_cast<int>(ret.error()));

    SUBCASE("First line")
    {
        CHECK(request.getMessage().method == "GET");
        CHECK(request.getMessage().requestTarget == "/index.html");
        CHECK(request.getMessage().protocol == "HTTP/1.1");
    }
    SUBCASE("Headers")
    {
        REQUIRE(request.getMessage().headers.contains("host"));
        CHECK(request.getMessage().headers["host"][0] == "localhost:8080");
        REQUIRE(request.getMessage().headers.contains("user-agent"));
        CHECK(request.getMessage().headers["user-agent"][0] == "curl/7.68.0");
        REQUIRE(request.getMessage().headers.contains("accept"));
        CHECK(request.getMessage().headers["accept"][0] == "*/*");
        REQUIRE(request.getMessage().headers.contains("content-length"));
        CHECK(request.getMessage().headers["content-length"][0] == "8");
    }
    SUBCASE("Body")
    {
        CHECK(request.getMessage().body == "abcdefgh");
    }
}

TEST_CASE("Basic chunked body test")
{
    HTTPRequest request{};
    std::string basic = "GET /index.html HTTP/1.1\r\n"
                        "Host: localhost:8080\r\n"
                        "User-Agent: curl/7.68.0\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Accept: */*\r\n"
                        "\r\n"
                        "8\r\n"
                        "abcdefgh\r\n"
                        "a\r\n"
                        "01234567890\r\n"
                        "A\r\n"
                        "01234567890\r\n";

    auto ret = request.newData(basic);
    REQUIRE_MESSAGE(ret.has_value(), "Error val: ", static_cast<int>(ret.error()));

    SUBCASE("First line")
    {
        CHECK(request.getMessage().method == "GET");
        CHECK(request.getMessage().requestTarget == "/index.html");
        CHECK(request.getMessage().protocol == "HTTP/1.1");
    }
    SUBCASE("Headers")
    {
        REQUIRE(request.getMessage().headers.contains("host"));
        CHECK(request.getMessage().headers["host"][0] == "localhost:8080");
        REQUIRE(request.getMessage().headers.contains("user-agent"));
        CHECK(request.getMessage().headers["user-agent"][0] == "curl/7.68.0");
        REQUIRE(request.getMessage().headers.contains("accept"));
        CHECK(request.getMessage().headers["accept"][0] == "*/*");
        REQUIRE(request.getMessage().headers.contains("transfer-encoding"));
        CHECK(request.getMessage().headers["transfer-encoding"][0] == "chunked");
    }
    SUBCASE("Body")
    {
        CHECK(request.getMessage().body == "abcdefgh0123456789001234567890");
    }
}

////////////////////////////////////////////////////////////////////////////////
// Invalid first line                                                         //
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Invalid target")
{
    HTTPRequest request{};
    std::string basic = "GET test/index.html HTTP/1.1\r\n\r\n";

    auto ret = request.newData(basic);
    REQUIRE(!ret.has_value());
    CHECK(ret.error() == ResponseStatusCode::kBadRequest);
}

TEST_CASE("Invalid version")
{
    HTTPRequest request{};
    std::string basic = "GET /index.html HTTP/1.0\r\n\r\n";

    auto ret = request.newData(basic);
    REQUIRE(!ret.has_value());
    CHECK(ret.error() == ResponseStatusCode::kHTTPVersionNotSupported);
}

////////////////////////////////////////////////////////////////////////////////
// Invalid headers                                                            //
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("invalid header")
{
    HTTPRequest request{};
    std::string basic = "GET /index.html HTTP/1.1\r\n"
                        "Host: localhost:8080\r\n"
                        "User-@gent: curl/7.68.0\r\n"
                        "Accept: */*\r\n"
                        "\r\n";

    auto ret = request.newData(basic);
    REQUIRE(!ret.has_value());
    CHECK(ret.error() == ResponseStatusCode::kBadRequest);
}

TEST_CASE("No host header")
{
    HTTPRequest request{};
    std::string basic = "GET /index.html HTTP/1.1\r\n"
                        "User-agent: curl/7.68.0\r\n"
                        "Accept: */*\r\n"
                        "\r\n";

    auto ret = request.newData(basic);
    REQUIRE(!ret.has_value());
    CHECK(ret.error() == ResponseStatusCode::kBadRequest);
}

////////////////////////////////////////////////////////////////////////////////
// Invalid body                                                               //
////////////////////////////////////////////////////////////////////////////////
