#include <SDL2pp/SDL2pp.hh>

#include "SimDefs.h"
#include "Sim.h"
#include "Network.h"
#include "Timer.h"
#include "Log.h"
#include "Ignore.h"

#include <exception>
#include <memory>
#include <atomic>
#include <thread>

using namespace AM;
using namespace AM::Server;

/** We delay for 1ms when possible to reduce our CPU usage. We can't trust the
   scheduler to come back to us after exactly 1ms though, so we need to give it
   some leeway. Picked .003 = 3ms as a reasonable number. Open for tweaking. */
constexpr double DELAY_MINIMUM_TIME_S = .003;

int inputThread(std::atomic<bool>* exitRequested)
{
    while (!(*exitRequested)) {
        std::string userInput = "";
        std::getline(std::cin, userInput);
        if (userInput == "exit") {
            *exitRequested = true;
        }
    }

    return 0;
}

int main(int argc, char** argv)
try {
    // SDL2 needs this signature for main, but we don't use the parameters.
    ignore(argc);
    ignore(argv);

    // Init SDL2 before doing anything else.
    SDL2pp::SDL sdl();
    SDLNet_Init();

    // Set up file logging.
    Log::enableFileLogging("Server.log");

    // Set up the network utility.
    Network network;

    // Set up the sim.
    Sim sim(network);

    // Spin up a thread to check for command line input.
    std::atomic<bool> exitRequested = false;
    std::thread inputThreadObj(inputThread, &exitRequested);

    LOG_INFO("Starting main loop.");

    // Prime the timers so they don't start at 0.
    sim.initTimer();
    network.initTimer();
    while (!exitRequested) {
        // Run the sim.
        sim.tick();

        // Send waiting messages.
        network.tick();

        // See if we have enough time left to sleep.
        double simTimeLeft = sim.getTimeTillNextIteration();
        double networkTimeLeft = network.getTimeTillNextHeartbeat();
        if ((simTimeLeft > DELAY_MINIMUM_TIME_S)
            && (networkTimeLeft > DELAY_MINIMUM_TIME_S)) {
            // We have enough time to sleep for a few ms.
            // Note: We try to delay for 1ms because the OS will generally end
            //       up delaying us for 1-3ms.
            SDL_Delay(1);
        }
    }

    inputThreadObj.join();

    return 0;
} catch (SDL2pp::Exception& e) {
    std::cerr << "Error in: " << e.GetSDLFunction() << std::endl;
    std::cerr << "  Reason:  " << e.GetSDLError() << std::endl;
    return 1;
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
