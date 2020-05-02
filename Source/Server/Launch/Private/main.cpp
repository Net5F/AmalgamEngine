#include <SDL2pp/SDL2pp.hh>
#include <SDL_thread.h>
#include "Message_generated.h"

#include "SharedDefs.h"
#include "Game.h"
#include "Network.h"
#include "Timer.h"

#include <exception>
#include <iostream>
#include <memory>
#include <atomic>

using namespace AM;
using namespace AM::Server;

int inputThread(void* inExitRequested)
{
    std::atomic<bool>* exitRequested = static_cast<std::atomic<bool>*>(inExitRequested);
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
    // Set up the network utility.
    Network network;

    // Set up our game.
    Game game(network);

    // Spin up a thread to check for command line input.
    std::atomic<bool> exitRequested = false;
    SDL_Thread* inputThreadPtr = SDL_CreateThread(inputThread, "User Input",
        (void*) &exitRequested);

    std::cout << "Starting main loop." << std::endl;
    Timer timer;
    while (!exitRequested) {
        // Calc the time delta.
        float deltaSeconds = timer.getDeltaSeconds();

        game.tick(deltaSeconds);
    }

    SDL_WaitThread(inputThreadPtr, NULL);

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
