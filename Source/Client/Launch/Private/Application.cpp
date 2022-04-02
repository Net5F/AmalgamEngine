#include "Application.h"
#include "Config.h"
#include "SharedConfig.h"
#include "Profiler.h"
#include "Paths.h"
#include "Log.h"

#include <SDL2/SDL.h>

#include <memory>
#include <functional>
#include "Ignore.h"

namespace AM
{
namespace Client
{
Application::Application()
: sdl(SDL_INIT_VIDEO)
, sdlWindow("Amalgam", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            Config::ACTUAL_SCREEN_WIDTH, Config::ACTUAL_SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN)
, sdlRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED)
, assetCache(sdlRenderer.Get())
, spriteData(assetCache)
, uiEventDispatcher()
, network()
, networkCaller(std::bind_front(&Network::tick, &network),
                SharedConfig::NETWORK_TICK_TIMESTEP_S, "Network", true)
, sim(uiEventDispatcher, network, spriteData)
, simCaller(std::bind_front(&Simulation::tick, &sim),
            SharedConfig::SIM_TICK_TIMESTEP_S, "Sim", false)
, worldSinks{sim.getWorld().worldSignals}
, userInterface(worldSinks, uiEventDispatcher, sdlRenderer.Get(), assetCache,
                spriteData)
, uiCaller(std::bind_front(&UserInterface::tick, &userInterface),
           Config::UI_TICK_TIMESTEP_S, "UserInterface", true)
, renderer(sdlRenderer.Get(), sim, userInterface,
           std::bind_front(&PeriodicCaller::getProgress, &simCaller))
, rendererCaller(std::bind_front(&Renderer::render, &renderer),
                 Renderer::FRAME_TIMESTEP_S, "Renderer", true)
, eventHandlers{this, &renderer, &userInterface, &sim}
, exitRequested(false)
{
    // Enable delay reporting.
    simCaller.reportDelays(Simulation::SIM_DELAYED_TIME_S);

    // Set up our event filter.
    SDL_SetEventFilter(&Application::filterEvents, this);
}

void Application::start()
{
    // Set up file logging.
    // TODO: This currently will do weird stuff if you have 2 clients open.
    //       If we need a temporary solution we can use PIDs, but the real
    //       solution will be to eventually use account IDs in the file name.
    Log::enableFileLogging("Client.log");

    // Set up profiling.
    Profiler::init();

    // Connect to the server (waits for connection response).
    sim.connect();

    // Prime the timers so they don't start at 0.
    simCaller.initTimer();
    uiCaller.initTimer();
    networkCaller.initTimer();
    rendererCaller.initTimer();
    while (!exitRequested) {
        // Let the sim process an iteration if it needs to.
        simCaller.update();

        // Let the UI widgets tick if they need to.
        uiCaller.update();

        // Send a heartbeat if necessary.
        networkCaller.update();

        // Let the renderer render if it needs to.
        rendererCaller.update();

        // If we have enough time, dispatch events.
        if (enoughTimeTillNextCall(DISPATCH_MINIMUM_TIME_S)) {
            dispatchOSEvents();
        }

        // If we have enough time, sleep.
        if (enoughTimeTillNextCall(SLEEP_MINIMUM_TIME_S)) {
            // We have enough time to sleep for a few ms.
            // Note: We try to delay for 1ms because the OS will generally end
            //       up delaying us for 1-3ms.
            SDL_Delay(1);
        }
    }
}

bool Application::handleOSEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_QUIT:
            exitRequested = true;
            return true;
    }

    return false;
}

void Application::dispatchOSEvents()
{
    // Dispatch all waiting SDL events.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Pass the event to each handler in order, stopping if it returns as
        // handled.
        for (OSEventHandler* handler : eventHandlers) {
            if (handler->handleOSEvent(event)) {
                break;
            }
        }
    }
}

bool Application::enoughTimeTillNextCall(double minimumTime)
{
    if ((simCaller.getTimeTillNextCall() > minimumTime)
        && (networkCaller.getTimeTillNextCall() > minimumTime)
        && (rendererCaller.getTimeTillNextCall() > minimumTime)) {
        return true;
    }
    else {
        return false;
    }
}

int Application::filterEvents(void* userData, SDL_Event* event)
{
    Application* app{static_cast<Application*>(userData)};
    ignore(event);
    ignore(app);

    // Currently no events that we care to filter.
    //
    // switch (event->type) {
    // }

    return 1;
}

} // End namespace Client
} // End namespace AM
