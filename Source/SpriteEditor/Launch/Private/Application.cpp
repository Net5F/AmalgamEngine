#include "Application.h"
#include "SimDefs.h"
#include "Profiler.h"
#include "Log.h"

#include "SDL.h"

#include <memory>
#include <functional>
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{
Application::Application(const std::string& runPath)
: sdl(SDL_INIT_VIDEO)
, sdlWindow("Amalgam Sprite Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN)
, sdlRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED)
, resourceManager(runPath, sdlRenderer)
, eventHandlers{this}
, exitRequested(false)
{
    // Uncomment to enable fullscreen.
    // window.SetFullscreen(SDL_WINDOW_FULLSCREEN);

    // Parse our config file.
    Configuration::init(runPath);

//    // Enable delay reporting.
//    simCaller.reportDelays(Simulation::SIM_DELAYED_TIME_S);

    // Set up our event filter.
    SDL_SetEventFilter(&Application::filterEvents, this);
}

void Application::start()
{
    // Set up file logging.
    Log::enableFileLogging("SpriteEditor.log");

    // Set up profiling.
    Profiler::init();

    // Prime the timers so they don't start at 0.
//    simCaller.initTimer();
//    networkCaller.initTimer();
//    rendererCaller.initTimer();
    while (!exitRequested) {
//        // Let the sim process an iteration if it needs to.
//        simCaller.update();
//
//        // Send a heartbeat if necessary.
//        networkCaller.update();
//
//        // Let the renderer render if it needs to.
//        rendererCaller.update();

        // If we have enough time, dispatch events.
        if (enoughTimeTillNextCall(DISPATCH_MINIMUM_TIME_S)) {
            dispatchEvents();
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

bool Application::handleEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_QUIT:
            exitRequested = true;
            return true;
    }

    return false;
}

void Application::dispatchEvents()
{
    // Dispatch all waiting SDL events.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Pass the event to each handler in order, stopping if it returns as
        // handled.
        for (EventHandler* handler : eventHandlers) {
            if (handler->handleEvent(event)) {
                break;
            }
        }
    }
}

bool Application::enoughTimeTillNextCall(double minimumTime)
{
    ignore(minimumTime);
//    if ((simCaller.getTimeTillNextCall() > minimumTime)
//        && (networkCaller.getTimeTillNextCall() > minimumTime)
//        && (rendererCaller.getTimeTillNextCall() > minimumTime)) {
//        return true;
//    }
//    else {
//        return false;
//    }
    return true;
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

} // End namespace SpriteEditor
} // End namespace AM
