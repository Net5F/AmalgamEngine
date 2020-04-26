#include <SDL2pp/SDL2pp.hh>
#include "Message_generated.h"

#include "SharedDefs.h"
#include "World.h"
#include "Game.h"
#include "RenderSystem.h"
#include "Network.h"
#include "Timer.h"

#include <exception>
#include <memory>
#include <iostream>
#include <iomanip>

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

    constexpr float RENDER_INTERVAL_S = 1 / 60.0f;
    float timeSinceRender = 0.0f;

    Timer timer;
    while (!exitRequested) {
        // Calc the time delta.
        float deltaSeconds = timer.getDeltaSeconds();

        // Run the game.
        game.tick(deltaSeconds);

        // Render at 60fps.
        timeSinceRender += deltaSeconds;
        if (timeSinceRender >= RENDER_INTERVAL_S) {
            if (timeSinceRender > 0.0171) {
                std::cout << "Render time: " << std::setprecision(10) << timeSinceRender << std::endl;
            }
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
