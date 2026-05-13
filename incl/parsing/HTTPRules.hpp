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
        std::string                                               absoluteRequestTarget;
        std::string                                               pathAfterLocation;
        std::string                                               protocol = "HTTP/1.1";
        std::unordered_map<std::string, std::vector<std::string>> headers;
        std::string                                               body;
};

enum class ResponseStatusCode
{
    kOK                      = 200,
    kCreated                 = 201,
    kNoContent               = 204,
    kBadRequest              = 400,
    kUnauthorized            = 401, // if we want access based on coockies/header values
    kForbidden               = 403, // if we want access based on coockies/header values
    kNotFound                = 404,
    kMethodNotAllowed        = 405,
    kRequestTimeout          = 408,
    kConflict                = 409,
    kContentTooLarge         = 413, // body larger than server limit, do we need this?
    kURITooLong              = 414, // maybe check if exeeds system path length including relative server root path?
    kUnsupportedMediaType    = 415,
    kImATeapot               = 418, // important
    kInternalServerError     = 500, // IDK if fork/execve crashes?
    kNotImplemented          = 501,
    kHTTPVersionNotSupported = 505,
};

inline const std::unordered_map<ResponseStatusCode, std::string> kStatusCodeStrings = {
    {ResponseStatusCode::kOK, "200 OK"},
    {ResponseStatusCode::kCreated, "201 Created"},
    {ResponseStatusCode::kNoContent, "204 No Content"},
    {ResponseStatusCode::kBadRequest, "400 Bad Request"},
    {ResponseStatusCode::kUnauthorized, "401 Unauthorized"},
    {ResponseStatusCode::kForbidden, "403 Forbidden"},
    {ResponseStatusCode::kNotFound, "404 Not Found"},
    {ResponseStatusCode::kMethodNotAllowed, "405 Method Not Allowed"},
    {ResponseStatusCode::kRequestTimeout, "408 Request Timeout"},
    {ResponseStatusCode::kConflict, "409 Conflict"},
    {ResponseStatusCode::kContentTooLarge, "413 Content Too Large"},
    {ResponseStatusCode::kURITooLong, "414 URI Too Long"},
    {ResponseStatusCode::kUnsupportedMediaType, "415 Unsupported Media Type"},
    {ResponseStatusCode::kImATeapot, "418 I'm a Teapot"},
    {ResponseStatusCode::kInternalServerError, "500 Internal Server Error"},
    {ResponseStatusCode::kNotImplemented, "501 Not Implemented"},
    {ResponseStatusCode::kHTTPVersionNotSupported, "505 HTTP Version Not Supported"},
};

class HTTPRules
{
    public:
        static auto statusCodeToString(ResponseStatusCode code) -> std::string;

    protected:
        static const std::string           delimiter_;
        static const std::set<std::string> supportedMethods_;
        static const std::string           HTTPVersion_;

        static auto is_tchar(const unsigned char& c) -> bool;
        static auto is_vchar(const unsigned char& c) -> bool;
        static auto is_obs_text(const unsigned char& c) -> bool;
        static auto is_field_vchar(const unsigned char& c) -> bool;
        static auto is_field_content(const unsigned char& c) -> bool;
        static auto is_field_value(const std::string& value) -> bool;

        static auto is_unreserved(const unsigned char& c) -> bool;
        static auto is_sub_delims(const unsigned char& c) -> bool;
        static auto is_hexdig(const unsigned char& c) -> bool;
        static auto is_segment(const std::string& str) -> bool;

        static auto is_absolute_path(const std::string& str) -> bool;
        static auto is_query(const std::string& str) -> bool;
        static auto is_origin_form(const std::string& str) -> bool;

        static auto validateRequestTarget(const std::string& str) -> std::expected<bool, ResponseStatusCode>;
        static auto validateProtocol(const std::string& str) -> std::expected<bool, ResponseStatusCode>;
        static auto validateHeader(const std::string& key, const std::string& value) -> bool;

    private:
};

#endif // HTTPRULES