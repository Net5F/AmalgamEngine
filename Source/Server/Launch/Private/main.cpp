#include <SDL2pp/SDL2pp.hh>
#include "messend.hpp"
#include "Message_generated.h"

#include "SharedDefs.h"
#include "InputComponent.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "SpriteComponent.h"
#include "MovementSystem.h"
#include "NetworkInputSystem.h"
#include "World.h"

#include <string>
#include <exception>
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <queue>
#include <algorithm>
#include <atomic>
#include <thread>

namespace AM
{
static constexpr Uint32 SCREEN_WIDTH = 1280;

static constexpr Uint32 SCREEN_HEIGHT = 720;
}

using namespace AM;

int main(int argc, char **argv)
try
{
    // Set up the SDL constructs.
    // TODO: Check if constructing this is necessary for networking.
    SDL2pp::SDL sdl();

    // Calc the center of the screen.
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2;

    // Set up our world.
    World world;

    // Set up our systems.
    NetworkInputSystem networkInputSystem(world);
    MovementSystem movementSystem(world);

    // Set up the networking.
    msnd::startup();
    msnd::Acceptor acceptor("127.0.0.1", 41499);
    std::vector<std::unique_ptr<msnd::Peer>> clients;

    // Spin up a thread to check for command line input.
    std::atomic<bool> bQuit = false;
    std::thread([&]
    {
        while (1) {
            std::string userInput = "";
            std::getline(std::cin, userInput);
            if (userInput == "exit") {
                bQuit = true;
            }
        }
    }).detach();

    std::cout << "Starting main loop." << std::endl;
    while (!bQuit) {
        // Check for new connections.
        std::unique_ptr<msnd::Peer> newClient = acceptor.accept();
        if (newClient != nullptr) {
            std::cout << "New peer connected." << std::endl;
            clients.push_back(std::move(newClient));

            // Build their entity.

            // Send them their ID.
        }

        // Check for disconnects.
        for (auto i = clients.begin(); i != clients.end(); ++i) {
            if (!((*i)->isConnected())) {
                clients.erase(i);
            }
        }

        // Will return Input::Type::Exit if the app needs to exit.
        networkInputSystem.processInputEvents();

        movementSystem.processMovements();

        SDL_Delay(1);
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
