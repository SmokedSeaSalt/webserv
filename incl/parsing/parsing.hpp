#include <string>
#include <unordered_map>
#include <vector>

struct HTTPMessage
{
	std::string method;
	std::string requestTarget;
	std::string protocol;
	std::unordered_map<std::string, std::vector<std::string>> headers;
	std::string body;
};

auto validateMethod(std::string str) -> std::expected<bool, std::string>;
//https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#request_targets
auto validateRequestTarget(std::string str) -> std::expected<std::string, std::string>;
auto validateProtocol(std::string str) -> std::expected<bool, std::string>;
auto validateHeader(std::string str) -> std::expected<bool, std::string>;

auto is_tchar(char c) -> bool;
