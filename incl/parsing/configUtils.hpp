#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP

#include "configParsing.hpp"
#include "connection.hpp"
#include <string>

namespace Config
{

auto getServerBlock(Config& config, std::tuple<std::string, int>& listenSocketIpPortPair) -> std::expected<ServerBlock, std::string>;
auto getLocation(ServerBlock& serverBlock, std::string target) -> std::expected<Location, std::string>;

} // namespace Config
#endif