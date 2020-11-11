#include "SDL.h"
#include "SDL2pp/SDL.hh"
#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"
#include "SDL2pp/Exception.hh"

#include "GameDefs.h"
#include "World.h"
#include "Game.h"
#include "RenderSystem.h"
#include "Network.h"
#include "Timer.h"
#include "Log.h"
#include "Ignore.h"

#include <exception>
#include <memory>
#include <atomic>

using namespace AM;
using namespace AM::Client;

/** We delay for 1ms when possible to reduce our CPU usage. We can't trust the
   scheduler to come back to us after exactly 1ms though, so we need to give it
   some leeway. Picked .003 = 3ms as a reasonable number. Open for tweaking. */
constexpr double DELAY_LEEWAY_S = .003;

int main(int argc, char** argv)
try {
    // SDL2 needs this signature for main, but we don't use the parameters.
    ignore(argc);
    ignore(argv);

    // Set up the SDL constructs.
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("Amalgam", SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
                          SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
    std::shared_ptr<SDL2pp::Texture> sprites
        = std::make_shared<SDL2pp::Texture>(renderer,
                                            "Resources/u4_tiles_pc_ega.png");

    // Uncomment to enable fullscreen.
    //    window.SetFullscreen(SDL_WINDOW_FULLSCREEN);

    // Set up file logging.
    // TODO: This currently will do weird stuff if you have 2 clients open.
    //       If we need a temporary solution we can use PIDs, but the real
    //       solution will be to eventually use account IDs in the log name.
    //    Log::enableFileLogging("Client.log");

    // Set up the network utility.
    Network network;

    // Set up the sim.
    Game game(network, sprites);

    // Set up the rendering system.
    RenderSystem renderSystem(renderer, game, window);

    // Connect to the server (waits for connection response).
    game.connect();

    // Get a pointer to the sim exit flag.
    // Triggered by things like window exit events.
    std::atomic<bool> const* exitRequested = game.getExitRequestedPtr();

    // Prime the timers so they don't start at 0.
    game.initTimer();
    network.initTimer();
    renderSystem.initTimer();
    while (!(*exitRequested)) {
        // Let the sim process an iteration if it needs to.
        game.tick();

        // Send a heartbeat if necessary.
        network.tick();

        // Let the render system render if it needs to.
        renderSystem.render();

        // TODO: Figure out a way to safely add delays, and test
        //       that the solution doesn't cause the client to fall behind the
        //       server.
        //        else if (((renderSystem.getAccumulatedTime() +
        //        executionSeconds + DELAY_LEEWAY_S)
        //                   < RenderSystem::RENDER_INTERVAL_S)
        //                 && ((game.getAccumulatedTime() + executionSeconds
        //                      + DELAY_LEEWAY_S)
        //                     < GAME_TICK_INTERVAL_S)) {
        //            // If we have enough time leftover to delay for a few ms,
        //            do it.
        //            // Note: We try to delay for 1ms because it will generally
        //            end up
        //            //       delaying for 1-2ms.
        //            SDL_Delay(1);
        //        }
    }

    return 0;
} catch (SDL2pp::Exception& e) {
    LOG_INFO("Error in: %s  Reason:  %s", e.GetSDLFunction(), e.GetSDLError());
    return 1;
} catch (std::exception& e) {
    LOG_INFO("%s", e.what());
    return 1;
}
