#include "Application.h"
#include "SharedConfig.h"
#include "Log.h"

#include "tracy/Tracy.hpp"
#include <functional>
#include <string>
#include <iostream>

namespace AM
{
namespace WorldServer
{
Application::Application()
: sdl{0}
, networkEventDispatcher{}
, userConfigInitializer{}
, resourceData{}
, graphicData{resourceData.get()}
, iconData{resourceData.get()}
, itemData{}
, castableData{graphicData}
, messageProcessorContext{networkEventDispatcher}
, network{messageProcessorContext}
, simulationContext{simulation,  network,  networkEventDispatcher,
                    graphicData, iconData, itemData,
                    castableData}
, simulation{simulationContext}
, networkCaller{std::bind_front(&WorldClientEndpoint::tick,
                                &network.clientEndpoint),
                SharedConfig::SERVER_NETWORK_TICK_TIMESTEP_S, "Network", true}
, simCaller{std::bind_front(&Simulation::tick, &simulation),
            SharedConfig::SIM_TICK_TIMESTEP_S, "Sim", false}
, callerGroup{{&simCaller, &networkCaller}, "Main loop"}
, exitRequested{false}
{
    // Initialize the global timer.
    Timer::getGlobalTime();

    // Enable delay reporting.
    simCaller.reportDelays(Simulation::SIM_DELAYED_TIME_S);
}

void Application::start()
{
    tracy::SetThreadName("ServerMain");

    // If any of the extensions aren't registered, exit early.
    if (!messageProcessorExtension || !simulationExtension) {
        LOG_FATAL("All extensions must be registered before calling start()");
    }

    // Start the asynchronous network thread.
    network.start();

    LOG_INFO("Starting main loop.");

    // Prime the timers so they don't start at 0.
    callerGroup.initTimers();

    while (!exitRequested) {
        callerGroup.recordIteration();

        // Let the simulation process an iteration if it needs to.
        simCaller.update();

        // Send any queued messages, if necessary.
        networkCaller.update();

        callerGroup.waitForNextCall();
    }
}

} // End namespace WorldServer
} // End namespace AM
