#include "Application.h"
#include "Config.h"
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
Application::Application()
: sdl(SDL_INIT_VIDEO)
, sdlWindow("Amalgam Sprite Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, SDL_WINDOW_SHOWN)
, sdlRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED)
, userInterface(sdlRenderer.Get())
, renderer(sdlRenderer, sdlWindow, userInterface)
, rendererCaller(std::bind(&Renderer::render, &renderer),
                 Renderer::FRAME_TIMESTEP_S, "Renderer", true)
, eventHandlers{this, &renderer}
, exitRequested(false)
{
    // Set fullscreen mode.
    switch (Config::FULLSCREEN_MODE) {
        case 0:
            sdlWindow.SetFullscreen(0);
            break;
        case 1:
            sdlWindow.SetFullscreen(SDL_WINDOW_FULLSCREEN);
            break;
        case 2:
            sdlWindow.SetFullscreen(SDL_WINDOW_FULLSCREEN_DESKTOP);
            break;
        default:
            LOG_ERROR("Invalid fullscreen value: %d", Config::FULLSCREEN_MODE);
    }

    // Set scaling quality.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, Config::SCALING_QUALITY);

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
    rendererCaller.initTimer();
    while (!exitRequested) {
        // Let the renderer render if it needs to.
        rendererCaller.update();

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
