#include "SDL2pp/SDL2pp.hh"
#include "Message_generated.h"

#include "GameDefs.h"
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

// We delay for 1ms when possible to reduce our CPU usage. We can't trust the scheduler
// to come back to us after exactly 1ms though, so we need to give it some leeway.
// Picked .003 = 3ms as a reasonable number. Open for tweaking.
constexpr float DELAY_LEEWAY_S = .003;

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

    Network network;
    Game game(network, sprites);
    RenderSystem renderSystem(renderer, game, window);
    Timer timer;

    std::atomic<bool> const* exitRequested = game.getExitRequestedPtr();

    // Connect to the server (waits for connection response).
    game.connect();

    // Prime the timer so it doesn't start at 0.
    timer.getDeltaSeconds(true);
    while (!(*exitRequested)) {
        // Calc the time delta.
        float deltaSeconds = timer.getDeltaSeconds(true);

        // Run the game.
        game.tick(deltaSeconds);

        // Render at 60fps.
        renderSystem.render(deltaSeconds);

        /* Act based on how long this tick took. */
        float executionSeconds = timer.getDeltaSeconds(false);
        if (executionSeconds >= RenderSystem::RENDER_INTERVAL_S) {
            DebugInfo("Overran the render tick rate.");
        }
        else if (((renderSystem.getAccumulatedTime() + executionSeconds + DELAY_LEEWAY_S)
                   < RenderSystem::RENDER_INTERVAL_S)
                 && ((game.getAccumulatedTime() + executionSeconds
                      + DELAY_LEEWAY_S)
                     < Game::GAME_TICK_INTERVAL_S)) {
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
