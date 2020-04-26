#include <SDL2pp/SDL2pp.hh>
#include "Message_generated.h"

#include "SharedDefs.h"
#include "InputComponent.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "SpriteComponent.h"
#include "World.h"
#include "Game.h"
#include "PlayerInputSystem.h"
#include "MovementSystem.h"
#include "NetworkMovementSystem.h"
#include "RenderSystem.h"

#include "Network.h"

#include <string>
#include <exception>
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <queue>
#include <algorithm>

using namespace AM;
using namespace AM::Client;

bool exitRequested = false;

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

    // Construct the server (Connected in Game.connect().)
    Network network;

    Game game(network, sprites);
    game.connect();

    Uint32 timeElapsed = 0;
    Uint32 lastFrameTimeElapsed = 0;
    float timeSinceRender = 0;
    constexpr float RENDER_INTERVAL_S = 1 / 60.0f;
    while (!exitRequested) {
        Uint32 start = SDL_GetTicks();
        // Calc the time delta.
        timeElapsed = SDL_GetTicks();
        float deltaSeconds = (timeElapsed - lastFrameTimeElapsed) / (float )1000;
        lastFrameTimeElapsed = timeElapsed;

        // Run the game.
        game.tick(deltaSeconds);

        // Render at 60fps.
        timeSinceRender += deltaSeconds;
        if (timeSinceRender >= RENDER_INTERVAL_S) {
            renderer.Clear();

            /* Render all entities. */
            World& world = game.getWorld();
            for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
                if (world.entityExists(entityID)) {
                    renderer.Copy(*(world.sprites[entityID].texturePtr),
                        world.sprites[entityID].posInTexture,
                        world.sprites[entityID].posInWorld);
                }
            }

            renderer.Present();

            timeSinceRender = 0;
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
