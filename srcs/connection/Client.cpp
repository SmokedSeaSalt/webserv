#include "Client.hpp"
#include "ConnectionManager.hpp"
#include "Execution.hpp"
#include "logging.hpp"
#include <string>

Client::Client(int socketfd, int listenSocketPort, std::string service, std::string host) : socketfd_(socketfd), listenSocketPort_(listenSocketPort), service_(service), host_(host), request_(), response_()
{
    this->state_ = ClientState::Receiving;
    this->error_ = ErrorType::None;
};

auto Client::handleEvent(const epoll_event& epollEvent) -> HandleEventResult
{

    int      fd     = epollEvent.data.fd;
    uint32_t events = epollEvent.events;

    switch (this->state_)
    {
    case ClientState::Receiving:
    {
        if (!(events & EPOLLIN))
            break;
        auto handleReceivingEventResult = ConnectionManager::handleReceivingEvent(fd);
        if (!handleReceivingEventResult.has_value())
        {
            LOG(LogLevel::kDebug, "recv() returned <= 0 at fd: {}, closing connection.", fd);
            ConnectionManager::eraseClient(this->getSocketfd());
            return HandleEventResult::kError;
        }
        this->request_.newData(handleReceivingEventResult.value());
        if (this->request_.getState() != RequestState::KDone)
            break;
        LOG(LogLevel::kInfo, "Packet received from fd:{} with content:\n{}\n", fd, getHTTPMessageString(this->request_.getMessage()));
        this->response_ = Execution::execute(this->request_.getMessage());
        [[fallthrough]];
    }
    case ClientState::Processing:
    {
        if (this->response_.getSendState() == SendState::kReady)
            this->state_ = ClientState::Sending;
        break;
    }
    case ClientState::Sending:
    {
        if (!(events & EPOLLOUT))
            break;
        ConnectionManager::handleSendingEvent(fd, this->response_);
        if (this->response_.getSendState() == SendState::kDone)
            this->state_ = ClientState::Sent;
        break;
    }
    case ClientState::Sent:
    {
        if (this->response_.getKeepAlive() == false)
            ConnectionManager::eraseClient(this->getSocketfd());
        else
        {
            this->request_  = {};
            this->response_ = {};
            this->state_    = ClientState::Receiving;
            LOG(LogLevel::kDebug, "Client socket at fd: {} being kept alive", fd);
        }
        break;
    }
    case ClientState::Closed:
        break;
    case ClientState::Error:
        break;
    }
	return HandleEventResult::kSuccess;
}

auto Client::setSocketfd(int arg) -> void
{
    this->socketfd_ = arg;
}
auto Client::getSocketfd() -> int
{
    return this->socketfd_;
}

auto Client::setListenSocketPort(int arg) -> void
{
    this->listenSocketPort_ = arg;
}
auto Client::getListenSocketPort() -> int
{
    return this->listenSocketPort_;
}

auto Client::setService(std::string arg) -> void
{
    this->service_ = arg;
}
auto Client::getService() -> std::string
{
    return this->service_;
}

auto Client::setHost(std::string arg) -> void
{
    this->host_ = arg;
}
auto Client::getHost() -> std::string
{
    return this->host_;
}

auto Client::setState(ClientState arg) -> void
{
    this->state_ = arg;
}
auto Client::getState() -> ClientState
{
    return this->state_;
}