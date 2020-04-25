#include <SDL2pp/SDL2pp.hh>
#include "Message_generated.h"

#include "SharedDefs.h"
#include "InputComponent.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "SpriteComponent.h"
#include "World.h"
#include "PlayerInputSystem.h"
#include "MovementSystem.h"
#include "NetworkMovementSystem.h"
#include "RenderSystem.h"

#include "NetworkClient.h"

#include <string>
#include <exception>
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <queue>
#include <algorithm>

using namespace AM;

int main(int argc, char **argv)
try
{
    // Set up the SDL constructs.
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("Amalgam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
    std::shared_ptr<SDL2pp::Texture> sprites = std::make_shared<SDL2pp::Texture>(
    renderer, "Resources/u4_tiles_pc_ega.png");

    // Calc the center of the screen.
    int centerX = renderer.GetOutputWidth() / 2;
    int centerY = renderer.GetOutputHeight() / 2;

    // Set up our world.
    World world;

    // Connect to the server.
    NetworkClient network;
    while (!(network.connect())) {
        std::cerr << "Network failed to connect. Retrying." << std::endl;
    }

    // Wait for the player's ID from the server.
    BinaryBufferPtr responseBuffer = network.receive();
    while (responseBuffer == nullptr) {
        responseBuffer = network.receive();
        SDL_Delay(10);
    }

    // Get the player ID from the connection response.
    const fb::Message* message = fb::GetMessage(responseBuffer->data());
    if (message->content_type() != fb::MessageContent::ConnectionResponse) {
        std::cerr << "Expected ConnectionResponse but got something else." << std::endl;
    }
    auto connectionResponse = static_cast<const fb::ConnectionResponse*>(message->content());
    EntityID player = connectionResponse->entityID();

    // Set up our systems.
    PlayerInputSystem playerInputSystem(world, network);
    NetworkMovementSystem networkMovementSystem(world, network);
    MovementSystem movementSystem(world);
    RenderSystem renderSystem(world);

    // Set up our player.
    SDL2pp::Rect textureRect(0, 32, 16, 16);
    SDL2pp::Rect worldRect(connectionResponse->x(), connectionResponse->y(), 64, 64);

    world.AddEntity("Player", player);
    world.positions[player].x = connectionResponse->x();
    world.positions[player].y = connectionResponse->y();
    world.movements[player].maxVelX = 15;
    world.movements[player].maxVelY = 15;
    world.sprites[player].texturePtr = sprites;
    world.sprites[player].posInTexture = textureRect;
    world.sprites[player].posInWorld = worldRect;
    world.AttachComponent(player, ComponentFlag::Input);
    world.AttachComponent(player, ComponentFlag::Movement);
    world.AttachComponent(player, ComponentFlag::Position);
    world.AttachComponent(player, ComponentFlag::Sprite);
    world.registerPlayerID(player);

    bool bQuit = false;
    Uint64 previousTime = 0;
    double timeSinceTick = 0;
    double timeSinceRender = 0;
    while (!bQuit) {
        // Calc the time delta.
        Uint64 currentTime = SDL_GetPerformanceCounter();
        double deltaMs = (double)(((currentTime - previousTime) * 1000)
                         / ((double) SDL_GetPerformanceFrequency()));
        previousTime = currentTime;

        // Check if we should process this tick.
        timeSinceTick += deltaMs;
        if (timeSinceTick >= 33.3) {
            timeSinceTick = 0;

            // Will return Input::Type::Exit if the app needs to exit.
            Input input = playerInputSystem.processInputEvents();
            if (input.type == Input::Exit) {
                break;
            }

            // Run all systems.
            networkMovementSystem.processServerMovements();

            movementSystem.processMovements(33.3);
        }

        // Render at 60fps.
        timeSinceRender += deltaMs;
        if (timeSinceRender >= 16.66) {
            timeSinceRender = 0;

            renderer.Clear();

            /* Render all entities. */
            for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
                if (world.entityExists(entityID)) {
                    renderer.Copy(*(world.sprites[entityID].texturePtr),
                        world.sprites[entityID].posInTexture,
                        world.sprites[entityID].posInWorld);
                }
            }

            renderer.Present();
        }
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
