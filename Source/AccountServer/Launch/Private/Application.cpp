#include "Application.h"
#include "Timer.h"
#include "Log.h"
#include <SDL3/SDL.h>

namespace AM
{
namespace AccountServer
{
Application::Application()
: sdl{0}
, database{}
, ioContext{}
, network{ioContext, database}
{
    // Initialize the global timer.
    Timer::getGlobalTime();
}

void Application::start()
{
    network.start();
}

void Application::handleOSEvents()
{
    // Process all waiting SDL events.
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT: {
                // TODO: Stop the IO context
                return;
            }
        }
    }
}

} // End namespace AccountServer
} // End namespace AM
