#include <SDL2pp/SDL2pp.hh>

#include "GameDefs.h"
#include "Game.h"
#include "Network.h"
#include "Timer.h"
#include "Debug.h"
#include "Ignore.h"

#include <exception>
#include <memory>
#include <atomic>
#include <thread>

using namespace AM;
using namespace AM::Server;

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

int main(int argc, char **argv)
try
{
    // SDL2 needs this signature for main, but we don't use the parameters.
    ignore(argc);
    ignore(argv);

    // Init SDL2 before doing anything else.
    SDL2pp::SDL sdl();
    SDLNet_Init();

    // Set up the network utility.
    Network network;

    // Set up our game.
    Game game(network);

    // Spin up a thread to check for command line input.
    std::atomic<bool> exitRequested = false;
    std::thread inputThreadObj(inputThread, &exitRequested);

    DebugInfo("Starting main loop.");

    // Prime the timers so they don't start at 0.
    game.initTimer();
    network.initTimer();
    while (!exitRequested) {
        // Run the game.
        game.tick();

        // Send waiting messages.
        network.tick();
    }

    inputThreadObj.join();

    return 0;
}
catch (SDL2pp::Exception& e) {
    std::cerr << "Error in: " << e.GetSDLFunction() << std::endl;
    std::cerr << "  Reason:  " << e.GetSDLError() << std::endl;
    return 1;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
