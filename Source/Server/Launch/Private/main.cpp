#include <SDL2pp/SDL2pp.hh>
#include "Message_generated.h"

#include "SharedDefs.h"
#include "InputComponent.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "SpriteComponent.h"
#include "MovementSystem.h"
#include "NetworkInputSystem.h"
#include "World.h"
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
#include <thread>

using namespace AM;

static constexpr int BUILDER_BUFFER_SIZE = 512;

int main(int argc, char **argv)
try
{
    // Calc the center of the screen.
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2;

    // Set up our world.
    World world;

    // Set up the network utility.
    NetworkServer network;

    // Set up our systems.
    NetworkInputSystem networkInputSystem(world);
    MovementSystem movementSystem(world);

    flatbuffers::FlatBufferBuilder builder(BUILDER_BUFFER_SIZE);

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
        // Add any new connections.
        std::vector<std::shared_ptr<Peer>> newClients =
            network.acceptNewClients();
        for (std::shared_ptr<Peer> peer : newClients) {
            // Build their entity.
            EntityID newID = world.AddEntity("Player");

            // Send them their ID.
            builder.Clear();
            auto response = fb::CreateConnectionResponse(builder, newID, 0, 0);
            auto encodedMessage = fb::CreateMessage(builder,
                fb::MessageContent::ConnectionResponse, response.Union());
            builder.Finish(encodedMessage);

            Uint8* buffer = builder.GetBufferPointer();
            BinaryBufferSharedPtr message = std::make_shared<std::vector<Uint8>>(
            buffer, (buffer + builder.GetSize()));

            bool result = network.send(peer, message);
            if (!result) {
                std::cerr << "Failed to send response." << std::endl;
            }
        }

        // Check for disconnects.
        network.checkForDisconnections();

        // Run all systems.
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
