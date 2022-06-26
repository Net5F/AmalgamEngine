#include "Application.h"
#include "SharedConfig.h"
#include "Log.h"

#include "Tracy.hpp"
#include <functional>
#include <string>
#include <iostream>

namespace AM
{
namespace Server
{
Application::Application()
: sdl(0)
, sdlNetInit()
, spriteData()
, networkEventDispatcher()
, network(networkEventDispatcher)
, networkCaller(std::bind_front(&Network::tick, &network),
                SharedConfig::NETWORK_TICK_TIMESTEP_S, "Network", true)
, simulation(networkEventDispatcher, network, spriteData)
, simCaller(std::bind_front(&Simulation::tick, &simulation),
            SharedConfig::SIM_TICK_TIMESTEP_S, "Sim", false)
, exitRequested(false)
{
    // Enable delay reporting.
    simCaller.reportDelays(Simulation::SIM_DELAYED_TIME_S);
}

void Application::start()
{
    tracy::SetThreadName("ServerMain");

    LOG_INFO("Starting main loop.");

    // Prime the timers so they don't start at 0.
    simCaller.initTimer();
    networkCaller.initTimer();
    while (!exitRequested) {
        // Let the simulation process an iteration if it needs to.
        simCaller.update();

        // Send any queued messages, if necessary.
        networkCaller.update();

        // See if we have enough time left to sleep.
        double simTimeLeft{simCaller.getTimeTillNextCall()};
        double networkTimeLeft{networkCaller.getTimeTillNextCall()};
        if ((simTimeLeft > SLEEP_MINIMUM_TIME_S)
            && (networkTimeLeft > SLEEP_MINIMUM_TIME_S)) {
            // We have enough time to sleep for a few ms.
            // Note: We try to delay for 1ms because the OS will generally end
            //       up delaying us for 1-3ms.
            SDL_Delay(1);
        }
    }
}

} // End namespace Server
} // End namespace AM
