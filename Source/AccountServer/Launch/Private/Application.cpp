#include "Application.h"
#include "Config.h"
#include "Paths.h"
#include "SDLHelpers.h"
#include "Timer.h"
#include "Log.h"
#include <SDL3/SDL.h>
#include <functional>

namespace AM
{
namespace AccountServer
{
Application::Application()
: sdl{0}
, database{}
, networkIoContext{}
, clientManager{networkIoContext}
//, serverManager{}
//, chatManager{}
, exitRequested{false}
{
    // Initialize the global timer.
    Timer::getGlobalTime();
}

void Application::start()
{
    // clientManager, serverManager, and chatManager all use this context for 
    // their events. By running it, we run them all.
    networkIoContext.run();
}

} // End namespace AccountServer
} // End namespace AM
