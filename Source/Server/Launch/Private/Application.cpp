#include "Application.h"
#include "SimDefs.h"
#include "Profiler.h"
#include "Log.h"

#include <functional>
#include <string>
#include <iostream>

namespace AM
{
namespace Server
{

Application::Application()
: sdl(SDL_INIT_VIDEO)
, sdlNetInit()
, network()
, networkCaller(std::bind(&Network::tick, &network)
            , NETWORK_TICK_TIMESTEP_S, "Network", true)
, sim(network)
, simCaller(std::bind(&Simulation::tick, &sim)
            , SIM_TICK_TIMESTEP_S, "Sim", false)
, exitRequested(false)
{
    // Enable delay reporting.
    simCaller.reportDelays(Simulation::SIM_DELAYED_TIME_S);

    // Spin up the thread to check for command line input.
    inputThreadObj = std::thread(&Application::receiveCliInput, this);
}

Application::~Application()
{
    exitRequested = true;
    inputThreadObj.join();
}

void Application::start()
{
    // Set up file logging.
    Log::enableFileLogging("Server.log");

    // Set up profiling.
    Profiler::init();

    LOG_INFO("Starting main loop.");

    // Prime the timers so they don't start at 0.
    simCaller.initTimer();
    networkCaller.initTimer();
    while (!exitRequested) {
        // Let the sim process an iteration if it needs to.
        simCaller.update();

        // Send a heartbeat if necessary.
        networkCaller.update();

        // See if we have enough time left to sleep.
        double simTimeLeft = simCaller.getTimeTillNextCall();
        double networkTimeLeft = networkCaller.getTimeTillNextCall();
        if ((simTimeLeft > DELAY_MINIMUM_TIME_S)
            && (networkTimeLeft > DELAY_MINIMUM_TIME_S)) {
            // We have enough time to sleep for a few ms.
            // Note: We try to delay for 1ms because the OS will generally end
            //       up delaying us for 1-3ms.
            SDL_Delay(1);
        }
    }
}

void Application::receiveCliInput()
{
    while (!exitRequested) {
        std::string userInput = "";
        std::getline(std::cin, userInput);
        if (userInput == "exit") {
            exitRequested = true;
        }
    }
}

} // End namespace Server
} // End namespace AM
