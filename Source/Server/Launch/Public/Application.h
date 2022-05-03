#pragma once

#include "Network.h"
#include "Simulation.h"
#include "SpriteData.h"
#include "PeriodicCaller.h"
#include "SDLNetInitializer.h"

#include "SDL2pp/SDL.hh"

#include <atomic>
#include <thread>

namespace AM
{
class EventDispatcher;

namespace Server
{
/**
 * Maintains the lifetime of all app modules (sim, network, etc) and manages
 * the main thread's loop.
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
    /** We sleep for 1ms when possible to reduce our CPU usage. We can't trust
        the scheduler to come back to us after exactly 1ms though, so we need
        to give it some leeway. Picked .003 = 3ms as a reasonable number. Open
        for tweaking. */
    static constexpr double SLEEP_MINIMUM_TIME_S = .003;

    //-------------------------------------------------------------------------
    // SDL Objects
    //-------------------------------------------------------------------------
    SDL2pp::SDL sdl;
    SDLNetInitializer sdlNetInit;

    //-------------------------------------------------------------------------
    // Modules, Dependencies, PeriodicCallers
    //-------------------------------------------------------------------------
    SpriteData spriteData;

    /** This is owned by the Application to follow the pattern of the Client's
        UI event dispatcher. */
    EventDispatcher networkEventDispatcher;

    Network network;
    PeriodicCaller networkCaller;

    Simulation sim;
    PeriodicCaller simCaller;

    //-------------------------------------------------------------------------
    // Additional, used during the loop
    //-------------------------------------------------------------------------
    /** Flags when to end the application. */
    std::atomic<bool> exitRequested;
};

} // End namespace Server
} // End namespace AM
