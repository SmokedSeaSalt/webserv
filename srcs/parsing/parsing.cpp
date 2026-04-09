#include "parsing.hpp"

auto to_lower(std::string str) -> std::string
{
    for (auto& c : str)
    {
        c = std::tolower(static_cast<unsigned char>(c));
    }
     return str;
}