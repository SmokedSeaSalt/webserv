#include <string>
#include <unordered_map>
#include <vector>

struct HTTPMessage
{
	std::string buffer;
	std::string method;
	std::string version;
	std::unordered_map<std::string, std::vector<std::string>> headers;
};

