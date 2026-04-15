#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../incl/doctest.h"
#include "HTTPResponse.hpp"
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// Basic                                                                      //
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Basic test")
{
    HTTPResponse response{};
    std::string basic = "HTTP/1.1 200 OK\r\n"
                        "Host: localhost:8080\r\n"
                        "User-Agent: curl/7.68.0\r\n"
                        "Accept: */*\r\n"
                        "\r\n";

	response.setProtocol("HTTP/1.1");
	response.addHeaderValue("Host", "localhost:8080");
	response.addHeaderValue("User-Agent", "curl/7.68.0");
	response.addHeaderValue("Accept", "*/*");

	SUBCASE("Normal response")
	{
		CHECK(response.createPacket().contains("HTTP/1.1 200 OK"));
		CHECK(response.createPacket().contains("host: localhost:8080"));
		CHECK(response.createPacket().contains("user-agent: curl/7.68.0"));
		CHECK(response.createPacket().contains("accept: */*"));
		CHECK(response.createPacket().contains("date:"));
		CHECK(response.createPacket().contains("server: webserv"));
	}

	SUBCASE("Error response")
	{
		CHECK(response.createErrorPacket(ResponseStatusCode::kNotFound).contains("HTTP/1.1 404 Not Found"));
		CHECK(response.createErrorPacket(ResponseStatusCode::kNotFound).contains("host: localhost:8080"));
		CHECK(response.createErrorPacket(ResponseStatusCode::kNotFound).contains("user-agent: curl/7.68.0"));
		CHECK(response.createErrorPacket(ResponseStatusCode::kNotFound).contains("accept: */*"));
		CHECK(response.createErrorPacket(ResponseStatusCode::kNotFound).contains("date:"));
		CHECK(response.createErrorPacket(ResponseStatusCode::kNotFound).contains("server: webserv"));
	}
}