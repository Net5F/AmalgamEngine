#include <SDL2pp/SDL2pp.hh>
#include <SDL_thread.h>
#include "Message_generated.h"

#include "SharedDefs.h"
#include "InputComponent.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "SpriteComponent.h"
#include "MovementSystem.h"
#include "NetworkInputSystem.h"
#include "World.h"
#include "Game.h"
#include "NetworkServer.h"

#include <string>
#include <exception>
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <queue>
#include <algorithm>
#include <atomic>

using namespace AM;

int inputThread(void* inExitRequested)
{
    while (1) {
        std::string userInput = "";
        std::getline(std::cin, userInput);
        if (userInput == "exit") {
            int* exitRequested = (int*) inExitRequested;
            *exitRequested = true;
        }
    }

    return 0;
}

int main(int argc, char **argv)
try
{
    // Calc the center of the screen.
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2;

    // Set up the network utility.
    NetworkServer network;

    // Set up our game.
    Game game(network);

    // Spin up a thread to check for command line input.
    std::atomic<bool> exitRequested = false;
    SDL_Thread* inputThreadPtr = SDL_CreateThread(inputThread, "User Input",
        (void*) &exitRequested);

    std::cout << "Starting main loop." << std::endl;
    Uint64 previousTime = 0;
    Uint64 currentTime = SDL_GetPerformanceCounter();
    while (!exitRequested) {
        double deltaMs = (double)(((currentTime - previousTime) * 1000)
                         / ((double) SDL_GetPerformanceFrequency()));
        game.tick(deltaMs);
    }

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
