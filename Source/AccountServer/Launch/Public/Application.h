#pragma once

#include "Database.h"
#include "Network.h"
#include "SDL_Wrappers/SDL.h"
#include "asio/io_context.hpp"

namespace AM
{
namespace AccountServer
{
/**
 * Maintains the lifetime of all app modules and manages the main thread's
 * loop.
 */
class Application
{
public:
    Application();

    /**
     * Begins the application. Assumes control of the thread until the
     * application exits.
     */
    void start();

private:
    void handleOSEvents();

    SDL sdl;

    /** The user account database. */
    Database database;

    /** This application's main event queue for network and OS events. */
    asio::io_context ioContext;

    Network network;
};

} // End namespace AccountServer
} // End namespace AM
