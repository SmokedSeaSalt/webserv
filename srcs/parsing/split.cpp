#include <sstream>
#include <string>
#include <vector>
#include <expected>

auto split(std::string line) -> std::expected<std::vector<std::string>, std::string>
{
	std::istringstream stream(line);
	std::vector<std::string> wordList;
	std::string token;

	while (std::getline(stream, token, ' ')) {
		if (!token.empty())
			wordList.push_back(token);
	}
}