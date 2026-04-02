#include <expected>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct HTTPMessage
{
        std::string                                               method;
        std::string                                               requestTarget;
        std::string                                               protocol;
        std::unordered_map<std::string, std::vector<std::string>> headers;
        std::string                                               body;
};

class HTTPRules
{
    public:
    protected:
        static const std::string           delimiter_;
        static const std::set<std::string> supportedMethods_;
        static const std::string           HTTPVersion_;

        static auto is_tchar(unsigned char c) -> bool;
        static auto is_vchar(unsigned char c) -> bool;
        static auto is_obs_text(unsigned char c) -> bool;
        static auto is_field_vchar(unsigned char c) -> bool;
        static auto is_field_content(unsigned char c) -> bool;
        static auto is_field_value(std::string value) -> bool;

        static auto validateMethod(std::string str) -> std::expected<bool, std::string>;
        static auto validateRequestTarget(std::string str)
            -> std::expected<std::string, std::string>;
        static auto validateProtocol(std::string str) -> std::expected<bool, std::string>;
        static auto validateHeader(std::string key, std::string value) -> bool;

    private:
};