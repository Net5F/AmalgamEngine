#pragma once

#include "OSEventHandler.h"
#include "PeriodicCaller.h"
#include "DataModel.h"

#include "SDL_Wrappers/SDL.h"

#include <atomic>

namespace AM
{
namespace AccountServer
{
/**
 * Maintains the lifetime of all app modules and manages the main thread's
 * loop.
 */
class Application : public OSEventHandler
{
public:
    Application();

    ~Application();

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

    /** The data model that holds the user's project data. */
    DataModel dataModel;

    //-------------------------------------------------------------------------
    // Data, Modules, Contexts
    //-------------------------------------------------------------------------
    ClientHandler clientHandler;

    ServerHandler serverHandler;

    ChatHandler chatHandler;

    //-------------------------------------------------------------------------
    // PeriodicCallers
    //-------------------------------------------------------------------------
    PeriodicCaller clientCaller;
    PeriodicCaller serverCaller;
    PeriodicCaller chatCaller;

    /** True if there has been a request to exit the program, else false. */
    std::atomic<bool> exitRequested;
};

} // End namespace AccountServer
} // End namespace AM
