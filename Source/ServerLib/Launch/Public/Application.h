#pragma once

#include "UserConfigInitializer.h"
#include "Network.h"
#include "Simulation.h"
#include "ResourceData.h"
#include "GraphicData.h"
#include "IconData.h"
#include "ItemData.h"
#include "CastableData.h"
#include "PeriodicCaller.h"
#include "SDLNetInitializer.h"
#include "IMessageProcessorExtension.h"
#include "MessageProcessorContext.h"
#include "ISimulationExtension.h"
#include "SimulationContext.h"

#include "entt/signal/dispatcher.hpp"

#include "SDL2pp/SDL.hh"

#include <atomic>
#include <thread>

namespace AM
{
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
    static constexpr double SLEEP_MINIMUM_TIME_S{.003};

    //-------------------------------------------------------------------------
    // SDL Objects
    //-------------------------------------------------------------------------
    SDL2pp::SDL sdl;

    SDLNetInitializer sdlNetInit;

    //-------------------------------------------------------------------------
    // Event Busses
    //-------------------------------------------------------------------------
    /** Used for Network -> Sim message events. */
    EventDispatcher networkEventDispatcher;

    //-------------------------------------------------------------------------
    // Data, Modules, Contexts
    //-------------------------------------------------------------------------
    UserConfigInitializer userConfigInitializer;

    ResourceData resourceData;
    GraphicData graphicData;
    IconData iconData;
    ItemData itemData;
    CastableData castableData;

    MessageProcessorContext messageProcessorContext;
    Network network;

    SimulationContext simulationContext;
    Simulation simulation;

    //-------------------------------------------------------------------------
    // PeriodicCallers
    //-------------------------------------------------------------------------
    /** Calls network.tick() at the network tick rate. */
    PeriodicCaller networkCaller;

    /** Calls simulation.tick() at the sim tick rate. */
    PeriodicCaller simCaller;

    //-------------------------------------------------------------------------
    // Module Extensions
    //-------------------------------------------------------------------------
    /** Contains the project's extension functions.
        Allows the project to provide code and have it be called at the 
        appropriate time.
        Note: This class guarantees that these extensions will be set and non-
              null before the loop starts running. This means we don't need to 
              null check the extension pointers in any class. */
    std::unique_ptr<IMessageProcessorExtension> messageProcessorExtension;
    std::unique_ptr<ISimulationExtension> simulationExtension;

    //-------------------------------------------------------------------------
    // Additional, used during the loop
    //-------------------------------------------------------------------------
    /** Flags when to end the application. */
    std::atomic<bool> exitRequested;
};

template<typename T>
void Application::registerMessageProcessorExtension()
{
    messageProcessorExtension = std::make_unique<T>(messageProcessorContext);
    network.setMessageProcessorExtension(messageProcessorExtension.get());
}

template<typename T>
void Application::registerSimulationExtension()
{
    simulationExtension = std::make_unique<T>(simulationContext);
    simulation.setExtension(simulationExtension.get());
}

} // End namespace Server
} // End namespace AM
