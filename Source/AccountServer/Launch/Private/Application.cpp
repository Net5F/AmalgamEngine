#include "Application.h"
#include "Config.h"
#include "Paths.h"
#include "SDLHelpers.h"
#include "Log.h"

#include <SDL3/SDL.h>
#include "nfd.hpp"

#include <memory>
#include <functional>

namespace AM
{
namespace AccountServer
{
Application::Application()
: sdl{SDL_INIT_VIDEO}
, dataModel{}
, clientHandler{}
, serverHandler{}
, chatHandler{} // TODO: Move number to constant
, clientCaller{std::bind_front(&ClientHandler::update, &clientHandler), 0.2,
               "ClientHandler", true}
, serverCaller{std::bind_front(&ServerHandler::update, &serverHandler), 0.2,
               "ServerHandler", true}
, chatCaller{std::bind_front(&ChatHandler::update, &chatHandler), 0.2,
             "ChatHandler", true}
, exitRequested{false}
{
    // Initialize the global timer.
    Timer::getGlobalTime();
}

Application::~Application() {}

void Application::start()
{
    // Prime the timers so they don't start at 0.
    clientCaller.initTimer();
    serverCaller.initTimer();
    chatCaller.initTimer();
    while (!exitRequested) {
        // Process messages, if necessary.
        clientCaller.processMessages();
        serverCaller.processMessages();
        chatCaller.processMessages();

        // See if we have enough time left to sleep.
        double clientTimeLeft{clientCaller.getTimeTillNextCall()};
        double serverTimeLeft{serverCaller.getTimeTillNextCall()};
        double chatTimeLeft{chatCaller.getTimeTillNextCall()};
        if ((clientTimeLeft > SLEEP_MINIMUM_TIME_S)
            && (serverTimeLeft > SLEEP_MINIMUM_TIME_S)
            && (chatTimeLeft > SLEEP_MINIMUM_TIME_S)) {
            // We have enough time to sleep for a few ms.
            // Note: We try to delay for 1ms because the OS will generally end
            //       up delaying us for 1-3ms.
            SDL_Delay(1);
        }
    }
}

} // End namespace AccountServer
} // End namespace AM
