#include "logging.hpp"
#include "signals.hpp"
#include <signal.h>

namespace Signals
{

void handle_signal(int signum)
{
    if (signum == SIGINT)
        shouldShutdown = true;
    LOG(LogLevel::kInfo, "Signal received: {}. shouldShutdown: {}", signum, shouldShutdown);
}

auto initSignals() -> int
{
    sigset_t         set;
    struct sigaction sa;

    if (sigemptyset(&set) == -1 || (sigaddset(&set, SIGINT) == -1))
    {
        LOG(LogLevel::kInfo, "Sigemptyset/sigaddset failed");
        return -1;
    }

    sa.sa_handler = &handle_signal;
    sa.sa_mask    = set;
    sa.sa_flags   = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        LOG(LogLevel::kInfo, "Sigaction failed");
        return -1;
    }
    return 0;
}

} // namespace Signals
