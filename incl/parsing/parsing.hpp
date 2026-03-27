#include <string>

struct HTTPMessage
{
	std::string buffer;
	std::string method;
	std::string version;
};