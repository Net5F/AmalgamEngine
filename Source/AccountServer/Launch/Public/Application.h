#pragma once

#include "ClientManager.h"
#include "Database.h"
#include "SDL_Wrappers/SDL.h"
#include "asio/io_context.hpp"
#include <atomic>

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
    /** The minimum "time to next call" required to trigger a main loop sleep.
        We sleep for 1ms when possible to reduce our CPU usage. We can't trust
        the scheduler to come back to us after exactly 1ms though, so we busy
        wait if something needs to be called soon.
        Higher value == more CPU usage. */
    static constexpr double SLEEP_MINIMUM_TIME_S{.003};

    SDL sdl;

    /** The user account database. */
    Database database;

    //-------------------------------------------------------------------------
    // Data, Modules, Contexts
    //-------------------------------------------------------------------------
    /** Shared network event queue for all managers. */
    asio::io_context networkIoContext;

    ClientManager clientManager;

    //ServerManager serverManager;

    //ChatManager chatManager;

    /** True if there has been a request to exit the program, else false. */
    std::atomic<bool> exitRequested;
};

} // End namespace AccountServer
} // End namespace AM
