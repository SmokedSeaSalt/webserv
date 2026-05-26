#include <expected>
#include <sstream>
#include <string>
#include <vector>

auto split(std::string line, char delimChar) -> std::expected<std::vector<std::string>, std::string>
{
    std::istringstream       stream(line);
    std::vector<std::string> wordList;
    std::string              token;

    while (std::getline(stream, token, delimChar))
    {
        if (!token.empty())
            wordList.push_back(token);
    }
    return wordList;
}