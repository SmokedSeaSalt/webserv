#include "configParsing.hpp"
#include <string>
#include <expected>
#include <fstream>
#include "parsing.hpp"

auto	parseLocation(std::ifstream& inFile) -> std::expected<Location, std::string>
{
	Location location;
	(void)inFile;
	return location;
}