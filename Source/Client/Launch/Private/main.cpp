#include <SDL2pp/SDL2pp.hh>
#include "Message_generated.h"

#include "SharedDefs.h"
#include "World.h"
#include "Game.h"
#include "RenderSystem.h"
#include "Network.h"
#include "Timer.h"
#include "Debug.h"

#include <exception>
#include <memory>
#include <atomic>

using namespace AM;
using namespace AM::Client;

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

    // Construct the network manager (connected in Game.connect()).
    Network network;

    Game game(network, sprites);
    game.connect();

    constexpr float RENDER_INTERVAL_S = 1 / 60.0f;
    float renderTimeAccumulator = 0.0f;

    // We delay for 1ms when possible to reduce our CPU usage. We can't trust the scheduler
    // to come back to us after exactly 1ms though, so we need to give it some leeway.
    // Picked .003 = 3ms as a reasonable number. Open for tweaking.
    constexpr float DELAY_LEEWAY_S = .003;

    Timer timer;
    // Prime the timer so it doesn't start at 0.
    timer.getDeltaSeconds(true);

    std::atomic<bool> const* exitRequested = game.getExitRequestedPtr();
    while (!(*exitRequested)) {
        // Calc the time delta.
        float deltaSeconds = timer.getDeltaSeconds(true);

        // Run the game.
        game.tick(deltaSeconds);

        // Render at 60fps.
        renderTimeAccumulator += deltaSeconds;
        if (renderTimeAccumulator >= RENDER_INTERVAL_S) {
            renderer.Clear();

            /* Render all entities. */
            World& world = game.getWorld();
            // How far we are between game ticks in decimal percent.
            float alpha = game.getTimeAccumulator() / Game::GAME_TICK_INTERVAL_S;
            for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
                if (world.entityExists(entityID)) {
                    const SpriteComponent& sprite = world.sprites[entityID];
                    const PositionComponent& position = world.positions[entityID];
                    const PositionComponent& oldPosition = world.oldPositions[entityID];

                    // Lerp'd position based on how far we are between game ticks.
                    // TODO: Have a real conversion instead of casting to int here.
                    int lerpX = (position.x * alpha) + (oldPosition.x * (1.0 - alpha));
                    int lerpY = (position.y * alpha) + (oldPosition.y * (1.0 - alpha));
                    SDL2pp::Rect spriteWorldData = { lerpX, lerpY, sprite.width,
                            sprite.height };

                    renderer.Copy(*(world.sprites[entityID].texturePtr),
                        world.sprites[entityID].posInTexture, spriteWorldData);
                }
            }

            renderer.Present();

            renderTimeAccumulator -= RENDER_INTERVAL_S;
            if (renderTimeAccumulator >= RENDER_INTERVAL_S) {
                // If we've accumulated enough time to render more, something
                // happened (probably a window event that stopped app execution.)
                // We still only want to render the latest data, but it's worth giving
                // debug output that we detected this.
                DebugInfo(
                    "Detected a delayed render. renderTimeAccumulator: %f. Setting to 0.",
                    renderTimeAccumulator);
                renderTimeAccumulator = 0;
            }
        }

        /* Act based on how long this tick took. */
        float executionSeconds = timer.getDeltaSeconds(false);
        if (executionSeconds >= RENDER_INTERVAL_S) {
            DebugInfo("Overran the render tick rate.");
        }
        else if ((renderTimeAccumulator + executionSeconds + DELAY_LEEWAY_S)
        < RENDER_INTERVAL_S) {
            // If we have enough time leftover to delay for 1ms, do it.
            SDL_Delay(1);
        }
    }

    return 0;
}
catch (SDL2pp::Exception& e) {
    DebugInfo("Error in: %s  Reason:  %s", e.GetSDLFunction(), e.GetSDLError());
    return 1;
}
catch (std::exception& e) {
    DebugInfo("%s", e.what());
    return 1;
}
