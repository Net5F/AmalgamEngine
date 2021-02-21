#include "Application.h"
#include "SimDefs.h"
#include "Log.h"

#include "SDL.h"

#include <memory>
#include <atomic>
#include <functional>

namespace AM
{
namespace Client
{

Application::Application()
: sdl(SDL_INIT_VIDEO)
, sdlWindow("Amalgam", SDL_WINDOWPOS_UNDEFINED,
          SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
          SDL_WINDOW_SHOWN)
, sdlRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED)
, network()
, networkCaller(std::bind(&Network::tick, &network)
            , NETWORK_TICK_TIMESTEP_S, "Network", true)
// TODO: Replace texture pointer with texture loader.
, sim(network, std::make_shared<SDL2pp::Texture>(sdlRenderer,
                                            "Resources/iso_test_sprites.png"))
, simCaller(std::bind(&Sim::tick, &sim)
            , SIM_TICK_TIMESTEP_S, "Sim", false)
, renderer(sdlRenderer, sdlWindow, sim, std::bind(&PeriodicCaller::getProgress, &simCaller))
, rendererCaller(std::bind(&Renderer::render, &renderer)
            , Renderer::RENDER_TICK_TIMESTEP_S, "Renderer", true)
{
    // Uncomment to enable fullscreen.
    //    window.SetFullscreen(SDL_WINDOW_FULLSCREEN);

    // Enable delay reporting.
    simCaller.reportDelays(Sim::SIM_DELAYED_TIME_S);
}

void Application::start()
{
    // Set up file logging.
    // TODO: This currently will do weird stuff if you have 2 clients open.
    //       If we need a temporary solution we can use PIDs, but the real
    //       solution will be to eventually use account IDs in the file name.
    Log::enableFileLogging("Client.log");

    // Connect to the server (waits for connection response).
    sim.connect();

    // Get a pointer to the sim exit flag.
    // Triggered by things like window exit events.
    std::atomic<bool> const* exitRequested = sim.getExitRequestedPtr();

    // Prime the timers so they don't start at 0.
    simCaller.initTimer();
    networkCaller.initTimer();
    rendererCaller.initTimer();
    while (!(*exitRequested)) {
        // Let the sim process an iteration if it needs to.
        simCaller.update();

        // Send a heartbeat if necessary.
        networkCaller.update();

        // Let the render system render if it needs to.
        rendererCaller.update();

        // See if we have enough time left to sleep.
        double simTimeLeft = simCaller.getTimeTillNextCall();
        double networkTimeLeft = networkCaller.getTimeTillNextCall();
        double renderTimeLeft = rendererCaller.getTimeTillNextCall();
        if ((simTimeLeft > DELAY_MINIMUM_TIME_S)
            && (networkTimeLeft > DELAY_MINIMUM_TIME_S)
            && (renderTimeLeft > DELAY_MINIMUM_TIME_S)) {
            // We have enough time to sleep for a few ms.
            // Note: We try to delay for 1ms because the OS will generally end
            //       up delaying us for 1-3ms.
            SDL_Delay(1);
        }
    }
}

} // End namespace Client
} // End namespace AM
