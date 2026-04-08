#include "../incl/HTTPRequest.hpp"
#include <iostream>

auto HTTPRequest::newData(std::string data) -> std::expected<void, std::string>
{
    std::cout << "Mock called" << std::endl;
    buffer_ += data;
    return {};
}

auto HTTPRequest::getBuffer() -> std::string
{
    return buffer_;
}