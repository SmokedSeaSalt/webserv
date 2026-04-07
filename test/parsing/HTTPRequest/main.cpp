#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../incl/doctest.h"
#include "HTTPRequest.hpp"
#include <iostream>

TEST_CASE("Basic test")
{
	HTTPRequest request{};
	std::string basic = "GET /index.html HTTP/1.1\r\n"
						"Host: localhost:8080\r\n"
						"User-Agent: curl/7.68.0\r\n"
						"Accept: */*\r\n"
						"\r\n";

	auto ret = request.newData(basic);
	if (!ret.has_value())
		std::cout << ret.error() << std::endl;
	REQUIRE(ret.has_value());

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
	while(i < basic.length())
	{
		auto ret = request.newData(basic.substr(i, 1));
		if (!ret.has_value())
			std::cout << ret.error() << std::endl;
		REQUIRE(ret.has_value());
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