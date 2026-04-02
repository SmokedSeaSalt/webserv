#include <string>
#include <expected>

class HTTPRequest
{
	public:
		auto newData(std::string data) -> std::expected<size_t, std::string>;
};