#ifndef SIGNALS_HPP
#define SIGNALS_HPP

namespace Signals
{

inline bool shouldShutdown = false;

auto handle_signal(int signum) -> void;
auto initSignals() -> int;

} // namespace Signals

#endif // SIGNALS_HPP