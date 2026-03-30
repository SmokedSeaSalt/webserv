#include <string>
#include <set>
#include <expected>
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

class HTTPRules
{
    public:

    protected:
        static const std::string delimiter_;
        static const std::set<std::string> supportedMethods_;
        static const std::string HTTPVersion_;

        static auto is_tchar(char c) -> bool;
        static auto is_vchar(char c) -> bool;
        
        static auto validateMethod(std::string str) -> std::expected<bool, std::string>;
        static auto validateRequestTarget(std::string str) -> std::expected<std::string, std::string>;
        static auto validateProtocol(std::string str) -> std::expected<bool, std::string>;
        static auto validateHeader(std::string key, std::string value) -> std::expected<bool, std::string>;

    private:
};


//https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#request_targets
auto validateRequestTarget(std::string str) -> std::expected<std::string, std::string>;
auto validateProtocol(std::string str) -> std::expected<bool, std::string>;
auto validateHeader(std::string key, std::string value) -> std::expected<bool, std::string>;

