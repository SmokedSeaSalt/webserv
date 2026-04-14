#include "configParsing.hpp"
#include "Execution.hpp"
#include <filesystem>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../incl/doctest.h"

TEST_CASE("Test execution with GET a file on the system")
{
	auto configResult = parseConfigFile("config.conf");
	REQUIRE(configResult.has_value());
	Config config = configResult.value();
	Execution execution(config);

	HTTPMessage httpMessage;
	httpMessage.method        = "GET";
	httpMessage.requestTarget = "helloWorld.html";
	httpMessage.protocol      = "HTTP/1.1";
	httpMessage.headers["host"] = {"localhost:8080"};
	httpMessage.headers["accept"] = {"text/html"};
	httpMessage.body = "";

	CHECK(httpMessage.method == "GET");
	CHECK(httpMessage.requestTarget == "helloWorld.html");
	CHECK(httpMessage.protocol == "HTTP/1.1");

	HTTPResponse repsonse = execution.execute(httpMessage);
	// CHECK(repsonse. == "");
}
