#include "Application.h"
#include "SharedConfig.h"
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
, networkCaller(std::bind_front(&Network::tick, &network),
                SharedConfig::NETWORK_TICK_TIMESTEP_S, "Network", true)
, sim(network)
, simCaller(std::bind_front(&Simulation::tick, &sim),
            SharedConfig::SIM_TICK_TIMESTEP_S, "Sim", false)
, exitRequested(false)
{
    // Enable delay reporting.
    simCaller.reportDelays(Simulation::SIM_DELAYED_TIME_S);

    // Spin up the thread to check for command line input.
    inputThreadObj = std::thread(&Application::receiveCliInput, this);
}

Application::~Application()
{
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
        if ((simTimeLeft > SLEEP_MINIMUM_TIME_S)
            && (networkTimeLeft > SLEEP_MINIMUM_TIME_S)) {
            // We have enough time to sleep for a few ms.
            // Note: We try to delay for 1ms because the OS will generally end
            //       up delaying us for 1-3ms.
            SDL_Delay(1);
        }
    }
}

void Application::receiveCliInput()
{
    // Block while waiting for user input.
    while (!exitRequested) {
        std::string userInput = "";
        std::getline(std::cin, userInput);
        if (userInput == "exit") {
            //  Note: For now, we only support this method of exiting.
            //        SDL2, by default, sends an SDL_QUIT event on SIGINT
            //        (CTRL+C), but we would need to have a non-blocking
            //        getline for this thread to shut down.
            exitRequested = true;
        }
    }
}

} // End namespace Server
} // End namespace AM
