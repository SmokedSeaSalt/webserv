#include "Client.hpp"
#include "Cgi.hpp"
#include "ConnectionManager.hpp"
#include "Execution.hpp"
#include "logging.hpp"
#include <string>

Client::Client(int socketfd, int listenSocketPort, std::tuple<std::string, int> listenSocketIpPortPair, std::string service, std::string host) : socketfd_(socketfd), listenSocketPort_(listenSocketPort), listenSocketIpPortPair_(listenSocketIpPortPair), service_(service), host_(host), request_(), response_()
{
    LOG(LogLevel::kInfo, "Listen socket ip={} port={} opened at: {}.", get<0>(listenSocketIpPortPair), get<1>(listenSocketIpPortPair), socketfd);

    this->state_        = ClientState::Receiving;
    this->error_        = ErrorType::None;
    this->requestIsCgi_ = false;
    this->updateLastActivityTime();
};

auto Client::handleEvent(const epoll_event& epollEvent) -> HandleEventResult
{

    int      fd     = epollEvent.data.fd;
    uint32_t events = epollEvent.events;

    switch (this->state_)
    {
    case ClientState::Receiving:
    {
        if (fd != this->socketfd_ || !(events & EPOLLIN))
            break;
        auto handleReceivingEventResult = ConnectionManager::handleReceivingEvent(fd);
        if (std::get<ssize_t>(handleReceivingEventResult) <= 0)
        {
            LOG(LogLevel::kDebug, "recv() returned {} at fd: {}, closing connection.", std::get<ssize_t>(handleReceivingEventResult), fd);
            ConnectionManager::closeConnection(this->getSocketfd());
            return HandleEventResult::kError;
        }
        this->updateLastActivityTime();
        auto newDataRes = this->request_.newData(std::get<std::string>(handleReceivingEventResult));
        if (!newDataRes.has_value())
        {
            LOG(LogLevel::kErrors, "Bad request from client fd:{}", fd);
            this->response_ = Execution::buildErrorResponse(*this, newDataRes.error());
            prepareResponseForSending();
        }
        if (this->request_.getState() != RequestState::KDone)
            break;
        LOG(LogLevel::kInfo, "Packet received from fd:{} with content:\n{}\n", fd, getHTTPMessageString(this->request_.getMessage()));

        execute();
        if (!this->requestIsCgi_)
            prepareResponseForSending();

        break;
    }
    case ClientState::Processing:
    {
        if (this->requestIsCgi_ && fd == this->cgifd_)
        {
            if (CgiHandler_.handleEvent(epollEvent) == HandleEventResult::kError)
            {
                LOG(LogLevel::kErrors, "CGI Error. Creating internal server error packet.");
                this->response_ = Execution::buildErrorResponse(*this, ResponseStatusCode::kInternalServerError);
                prepareResponseForSending();
                break;
            }
            if (CgiHandler_.getState() == CgiState::KDone)
                prepareResponseForSending();
            break;
        }
        break;
    }
    case ClientState::Sending:
    {
        if (!(events & EPOLLOUT))
            break;
        ssize_t bytesSend = ConnectionManager::handleSendingEvent(fd, this->response_.getRemainingPacket());
        if (bytesSend < 0)
        {
            this->response_.setSendState(SendState::kFailed);
            LOG(LogLevel::kErrors, "Error during send on fd:{}", fd);
            return HandleEventResult::kError;
        }
        this->updateLastActivityTime();
        this->response_.incrementTotalBytesSent(bytesSend);
        if (bytesSend == 0 || this->response_.getRemainingPacketLen() == 0)
        {
            LOG(LogLevel::kInfo, "Response finished sending on fd:{}", fd);
            this->response_.setSendState(SendState::kDone);
            processKeepAlive();
            break;
        }

        break;
    }
    }
    return HandleEventResult::kSuccess;
}

auto Client::prepareResponseForSending() -> void
{
    HTTPMessage requestMessage = this->getRequest().getMessage();

    if ((requestMessage.headers.contains("connection") && requestMessage.headers.at("connection")[0] == "close") || this->response_.getStatusCode() == ResponseStatusCode::kBadRequest)
    {
        this->response_.setKeepAlive(false);
        this->response_.setHeader("connection", "close");
    }
    else
    {
        this->response_.setKeepAlive(true);
        this->response_.setHeader("connection", "keep-alive");
    }

    if (this->getRequest().getMessage().method == "HEAD")
        this->getResponse().setBody("");

    std::string packet = this->response_.createPacket();
    LOG(LogLevel::kInfo, "Packet created for fd:{}", this->socketfd_);
    LOG(LogLevel::kVerbose, "with content:\n{}\n", packet);

    this->response_.setPacket(packet);
    this->response_.setSendState(SendState::kSending);
    this->state_ = ClientState::Sending;
}

auto Client::execute() -> void
{
    auto setupResult = Execution::setupRequestForExecution(*this);
    if (!setupResult.has_value())
    {
        this->response_ = setupResult.error();
        return;
    }

    if (this->requestIsCgi_)
    {
        auto CgiInitRet = this->CgiHandler_.init(shared_from_this());
        if (!CgiInitRet.has_value())
        {
            this->response_     = Execution::buildErrorResponse(*this, CgiInitRet.error());
            this->requestIsCgi_ = false;
            return;
        }
        this->cgifd_ = CgiInitRet.value();
        this->state_ = ClientState::Processing;
    }
    else if (this->getRequest().getLocation().cgiPaths.empty())
    {
        this->response_ = Execution::executeNonCGI(*this);
    }
    else
    {
        this->response_ = Execution::buildErrorResponse(*this, ResponseStatusCode::kForbidden);
    }
    return;
}

/// @brief checks if the connection should be closed
/// @return
auto Client::processKeepAlive() -> void
{
    if (this->response_.getKeepAlive() == false)
        ConnectionManager::closeConnection(this->getSocketfd());
    else
    {
        this->request_  = {};
        this->response_ = {};
        this->state_    = ClientState::Receiving;
        LOG(LogLevel::kDebug, "Client socket at fd: {} being kept alive", this->socketfd_);
    }
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
auto Client::getState() -> ClientState&
{
    return this->state_;
}

auto Client::setRequest(HTTPRequest arg) -> void
{
    this->request_ = arg;
}
auto Client::getRequest() -> HTTPRequest&
{
    return this->request_;
}

auto Client::setResponse(HTTPResponse arg) -> void
{
    this->response_ = arg;
}
auto Client::getResponse() -> HTTPResponse&
{
    return this->response_;
}

auto Client::setListenSocketIpPortPair(std::tuple<std::string, int> arg) -> void
{
    this->listenSocketIpPortPair_ = arg;
}
auto Client::getListenSocketIpPortPair() -> std::tuple<std::string, int>
{
    return this->listenSocketIpPortPair_;
}

auto Client::setRequestIsCgi(bool isCgi) -> void
{
    this->requestIsCgi_ = isCgi;
}
auto Client::getRequestIsCgi() -> bool
{
    return this->requestIsCgi_;
}

auto Client::setCgiPID(int pid) -> void
{
    this->cgiPID_ = pid;
}

auto Client::getCgiPID() -> int
{
    return this->cgiPID_;
}

void Client::updateLastActivityTime()
{
    lastActivityTime_ = std::chrono::steady_clock::now();
}

std::chrono::steady_clock::time_point Client::getLastActivity()
{
    return lastActivityTime_;
}