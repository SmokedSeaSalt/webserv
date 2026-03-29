#include "parsing/HTTPRequest.hpp"

auto HTTPRequest::newData(std::string data) -> std::expected<size_t, std::string>
{
    this->buffer_ += data;
    
    std::string::size_type pos = this->buffer_.find(this->delimiter_);
    if (pos == std::string::npos)
        return (0);

    switch (this->state_)
    {
        case (RequestState::kStartLine):
        {
            const auto ret = parseStartLine(this->buffer_.substr(0, pos + this->delimiter_.size()));
            if (!ret.has_value())
                return std::unexpected(ret.error());

            this->state_ = RequestState::kHeaders;
            break;
        }
        case (RequestState::kHeaders):
        {
            if (pos == 0)
            {
                if (this->message_.method == "PATCH" || this->message_.method == "POST" || this->message_.method == "PUT")
                    this->state_ = RequestState::kBody;
                else
                    this->state_ = RequestState::KDone;
                break;
            }
            const auto ret = parseStartLine(this->buffer_.substr(0, pos + this->delimiter_.size()));
            if (!ret.has_value())
                return std::unexpected(ret.error());
            




            break;
        }
        case (RequestState::kBody):
        {

            break; 
        }
        case (RequestState::KDone):
        {

            break;
        }
    }
    
    this->buffer_.erase(pos + this->delimiter_.size());
    return pos;

}

auto HTTPRequest::parseStartLine(std::string line) -> std::expected<size_t, std::string>
{

}

auto HTTPRequest::parseHeaders() -> std::expected<size_t, std::string>
{

}

auto HTTPRequest::parseBody() -> std::expected<size_t, std::string>
{

}
