#include "Application.h"
#include "Timer.h"
#include "asio/signal_set.hpp"
#include <csignal>

namespace AM
{
namespace AccountServer
{
Application::Application()
: sdl{0}
, database{}
, ioContext{}
, network{ioContext, database}
{
    // Initialize the global timer.
    Timer::getGlobalTime();
}

void Application::start()
{
    // Stop the event loop when the process receives a termination signal.
    asio::signal_set terminationSignals{ioContext, SIGINT, SIGTERM};
    terminationSignals.async_wait(
        [this](const asio::error_code& error, int) {
            if (!error) {
                ioContext.stop();
            }
        });

    network.start();
}

} // End namespace AccountServer
} // End namespace AM
