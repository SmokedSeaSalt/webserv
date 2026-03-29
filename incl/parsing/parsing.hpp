#include <string>
#include <unordered_map>
#include <vector>

struct HTTPMessage
{
	std::string method;
	std::string requestTarget;
	std::string protocol;
	std::string version;
	std::unordered_map<std::string, std::vector<std::string>> headers;
	std::string body;
};

