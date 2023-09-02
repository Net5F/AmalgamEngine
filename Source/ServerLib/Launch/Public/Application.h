#pragma once

#include "UserConfigInitializer.h"
#include "Network.h"
#include "Simulation.h"
#include "SpriteData.h"
#include "PeriodicCaller.h"
#include "SDLNetInitializer.h"
#include "IMessageProcessorExtension.h"
#include "MessageProcessorExDependencies.h"
#include "ISimulationExtension.h"
#include "SimulationExDependencies.h"

#include "SDL2pp/SDL.hh"

#include <atomic>
#include <thread>

namespace AM
{
class EventDispatcher;

namespace Server
{
/**
 * The start of all server application activity. Owns all of the application's
 * modules (Simulation, Network, etc).
 *
 * Also manages the main thread's loop, calling each module's update function
 * when appropriate.
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

    //-------------------------------------------------------------------------
    // Engine Extension Registration
    //-------------------------------------------------------------------------
    /**
     * Registers an extension class to be called by the MessageProcessor.
     *
     * Note: The extension class type T must derive from
     *       IMessageProcessorExtension and must implement a constructor of
     *       the form T(MessageProcessorExDependencies).
     */
    template<typename T>
    void registerMessageProcessorExtension();

    /**
     * Registers an extension class to be called by the Simulation.
     *
     * Note: The extension class type T must derive from ISimulationExtension
     *       and must implement a constructor of the form
     *       T(SimulationExDependencies).
     */
    template<typename T>
    void registerSimulationExtension();

private:
    /** The minimum "time to next call" required to trigger a main loop sleep.
        We sleep for 1ms when possible to reduce our CPU usage. We can't trust
        the scheduler to come back to us after exactly 1ms though, so we busy
        wait if something needs to be called soon.
        Higher value == more CPU usage. */
    static constexpr double SLEEP_MINIMUM_TIME_S = .003;

    //-------------------------------------------------------------------------
    // SDL Objects
    //-------------------------------------------------------------------------
    SDL2pp::SDL sdl;

    SDLNetInitializer sdlNetInit;

    //-------------------------------------------------------------------------
    // Modules, Dependencies, PeriodicCallers
    //-------------------------------------------------------------------------
    UserConfigInitializer userConfigInitializer;

    SpriteData spriteData;

    Network network;
    /** Calls network.tick() at the network tick rate. */
    PeriodicCaller networkCaller;

    Simulation simulation;
    /** Calls simulation.tick() at the sim tick rate. */
    PeriodicCaller simCaller;

    //-------------------------------------------------------------------------
    // Additional, used during the loop
    //-------------------------------------------------------------------------
    /** Flags when to end the application. */
    std::atomic<bool> exitRequested;
};

template<typename T>
void Application::registerMessageProcessorExtension()
{
    MessageProcessorExDependencies messageProcessorDeps{
        network.getEventDispatcher()};

    network.setMessageProcessorExtension(
        std::make_unique<T>(messageProcessorDeps));
}

template<typename T>
void Application::registerSimulationExtension()
{
    SimulationExDependencies simulationDeps{simulation, network, spriteData};

    simulation.setExtension(std::make_unique<T>(simulationDeps));
}

} // End namespace Server
} // End namespace AM
