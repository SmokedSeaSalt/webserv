#ifndef HTTPRULES
#define HTTPRULES

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

enum class ResponseStatusCode
{
    kOK = 200,
    kCreated = 201,
    kBadRequest = 400,
    kUnauthorized = 401, //if we want access based on coockies/header values
    kForbidden = 403, //if we want access based on coockies/header values
    kNotFound = 404,
    kMethodNotAllowed = 405,
    kRequestTimeout = 408,
    kContentTooLarge = 413, //body larger than server limit, do we need this?
    kURITooLong = 414, //maybe check if exeeds system path length including relative server root path?
    kUnsupportedMediaType = 415,
    kImATeapot = 418, //important
    kInternalServerError = 500, //IDK if fork/execve crashes?
    kNotImplemented = 501,
    kHTTPVersionNotSupported = 505,
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

        static auto is_unreserved(unsigned char c) -> bool;
        static auto is_sub_delims(unsigned char c) -> bool;
        static auto is_hexdig(unsigned char c) -> bool;
        static auto is_segment(std::string str) -> bool;

        static auto is_absolute_path(std::string str) -> bool;
        static auto is_query(std::string str) -> bool;
        static auto is_origin_form(std::string str) -> bool;

        static auto validateMethod(std::string str) -> std::expected<bool, ResponseStatusCode>;
        static auto validateRequestTarget(std::string str) -> std::expected<bool, ResponseStatusCode>;
        static auto validateProtocol(std::string str) -> std::expected<bool, ResponseStatusCode>;
        static auto validateHeader(std::string key, std::string value) -> bool;

    private:
};

#endif // HTTPRULES