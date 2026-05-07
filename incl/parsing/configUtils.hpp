#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP

#include "HTTPRequest.hpp"
#include "configParsing.hpp"
#include "connection.hpp"
#include <string>

namespace Config
{

auto getServerBlock(const std::tuple<std::string, int>& listenSocketIpPortPair) -> std::expected<ServerBlock, std::string>;
auto getLocation(ServerBlock& serverBlock, std::string target) -> std::expected<Location, std::string>;
auto checkLocationCompliance(HTTPRequest request) -> ResponseStatusCode;
auto checkContentLength(HTTPRequest request) -> ResponseStatusCode;

} // namespace Config
#endif