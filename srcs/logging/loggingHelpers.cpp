#include <string>
#include "HTTPRules.hpp"

std::string getHTTPMessageString(const HTTPMessage& msg)
{
	std::string result;
	// Start line: METHOD TARGET PROTOCOL\r\n
	result += msg.method + " " + msg.requestTarget + " " + msg.protocol + "\r\n";

	for (const auto& [key, values] : msg.headers) {
		for (const auto& value : values) {
			result += key + ": " + value + "\r\n";
		}
	}
	result += "\r\n";

	result += msg.body;

	return result;
}