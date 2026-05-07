#include "Client.hpp"
#include "Cgi.hpp"
#include "ConnectionManager.hpp"
#include "Execution.hpp"
#include "logging.hpp"
#include <string>

Client::Client(int socketfd, int listenSocketPort, std::tuple<std::string, int>& listenSocketIpPortPair, std::string service, std::string host) : socketfd_(socketfd), listenSocketPort_(listenSocketPort), listenSocketIpPortPair_(listenSocketIpPortPair), service_(service), host_(host), request_(), response_(), CgiHandler_(*this)
{
    this->state_        = ClientState::Receiving;
    this->error_        = ErrorType::None;
    this->requestIsCgi_ = false;
};

auto Client::handleEvent(const epoll_event& epollEvent) -> HandleEventResult
{

    int      fd     = epollEvent.data.fd;
    uint32_t events = epollEvent.events;

    switch (this->state_)
    {
    case ClientState::Receiving:
    {
        if (fd != this->socketfd_ && !(events & EPOLLIN))
            break;
        auto handleReceivingEventResult = ConnectionManager::handleReceivingEvent(fd);
        if (std::get<ssize_t>(handleReceivingEventResult) <= 0)
        {
            LOG(LogLevel::kDebug, "recv() returned <= 0 at fd: {}, closing connection.", fd);
            ConnectionManager::closeConnection(this->getSocketfd());
            return HandleEventResult::kError;
        }
        this->request_.newData(std::get<std::string>(handleReceivingEventResult));
        if (this->request_.getState() != RequestState::KDone)
            break;
        LOG(LogLevel::kInfo, "Packet received from fd:{} with content:\n{}\n", fd, getHTTPMessageString(this->request_.getMessage()));

        // TODO put this below in helper function until --delim--
        //also check if is cgi set in location in config
        this->requestIsCgi_ = Cgi::isRequestTargetCgi(this->request_.getMessage().requestTarget); // need absolute path? is it already set?
        if (this->requestIsCgi_)
        {
            auto CgiInitRet = this->CgiHandler_.init(); // Todo: error handling for this.
            if (!CgiInitRet.has_value())
            {
                this->response_     = CgiInitRet.error();
                this->requestIsCgi_ = false;
                break;
            }
            this->cgifd_ = CgiInitRet.value();
        }
        else
            this->response_ = Execution::execute(*this);
        // --delim--
        this->state_ = ClientState::Processing;
        break;
    }
    case ClientState::Processing:
    { // only handle cgi events here
        if (fd == this->cgifd_ && this->requestIsCgi_)
        {
            if (CgiHandler_.handleEvent(epollEvent) == HandleEventResult::kError) // TODO: error handling if fails should try to make internal server error to send to client
            {
                LOG(LogLevel::kErrors, "CGI Error. Creating internal server error packet.");
                this->response_.clearAllHeaders();
                std::string packet = this->response_.createPacket(ResponseStatusCode::kInternalServerError);
                LOG(LogLevel::kInfo, "Packet created for fd:{}", fd);
                LOG(LogLevel::kVerbose, "with content:\n{}\n", packet);

                this->response_.setPacket(packet);
                this->response_.setSendState(SendState::kSending);
                this->state_ = ClientState::Sending;
                break;
            }
            break;
        }
        if (this->response_.getSendState() == SendState::kReady)
        {
            std::string packet = this->response_.createPacket();
            LOG(LogLevel::kInfo, "Packet created for fd:{}", fd);
            LOG(LogLevel::kVerbose, "with content:\n{}\n", packet);

            this->response_.setPacket(packet);
            this->response_.setSendState(SendState::kSending);
            this->state_ = ClientState::Sending;
        }
        [[fallthrough]];
    }
    case ClientState::Sending:
    {
        if (!(events & EPOLLOUT))
            break;
        if (!(this->response_.getSendState() == SendState::kSending))
            break; //should never get in this state
        ssize_t bytesSend = ConnectionManager::handleSendingEvent(fd, this->response_.getRemainingPacket());

        if (bytesSend < 0)
        {
            this->response_.setSendState(SendState::kFailed);
            LOG(LogLevel::kErrors, "Error during send on fd:{}", fd);
            return HandleEventResult::kError;
            // todo handle error
        }
        this->response_.incrementTotalBytesSent(bytesSend);
        if (bytesSend == 0 || this->response_.getRemainingPacketLen() == 0)
        {
            LOG(LogLevel::kInfo, "Request finished sending on fd:{}", fd);
            this->response_.setSendState(SendState::kDone);
            this->state_ = ClientState::Sent;
            break;
        }

        break;
    }
    case ClientState::Sent:
    {
        if (this->response_.getKeepAlive() == false)
            ConnectionManager::closeConnection(this->getSocketfd());
        else
        {
            // Todo Reset CGI stuff
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