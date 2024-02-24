#pragma once

#include "OSEventHandler.h"
#include "UserConfigInitializer.h"
#include "Network.h"
#include "Simulation.h"
#include "Renderer.h"
#include "AssetCache.h"
#include "ResourceData.h"
#include "GraphicData.h"
#include "IconData.h"
#include "QueuedEvents.h"
#include "UserInterface.h"
#include "PeriodicCaller.h"
#include "IMessageProcessorExtension.h"
#include "MessageProcessorExDependencies.h"
#include "IRendererExtension.h"
#include "RendererExDependencies.h"
#include "ISimulationExtension.h"
#include "SimulationExDependencies.h"
#include "IUserInterfaceExtension.h"
#include "UserInterfaceExDependencies.h"

#include "SDL2pp/SDL.hh"
#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"

#include <atomic>
#include <functional>

namespace AM
{
namespace Client
{
/**
 * The start of all client application activity. Owns all of the application's
 * modules (Simulation, Network, etc).
 *
 * Also manages the main thread's loop, calling each module's update function
 * when appropriate.
 */
class Application : public OSEventHandler
{
public:
    Application();

    ~Application(){};

    /**
     * Begins the application. Assumes control of the thread until the
     * application exits.
     */
    void start();

    /**
     * Handles application-relevant events, such as exit requests.
     */
    bool handleOSEvent(SDL_Event& event) override;

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
     * Registers an extension class to be called by the Renderer.
     *
     * Note: The extension class type T must derive from IRendererExtension
     *       and must implement a constructor of the form
     *       T(RendererExDependencies).
     */
    template<typename T>
    void registerRendererExtension();

    /**
     * Registers an extension class to be called by the Simulation.
     *
     * Note: The extension class type T must derive from ISimulationExtension
     *       and must implement a constructor of the form
     *       T(SimulationExDependencies).
     */
    template<typename T>
    void registerSimulationExtension();

    /**
     * Registers an extension class to be called by the UserInterface.
     *
     * Note: The extension class type T must derive from IUserInterfaceExtension
     *       and must implement a constructor of the form
     *       T(UserInterfaceExDependencies).
     */
    template<typename T>
    void registerUserInterfaceExtension();

private:
    /** The minimum "time to next call" required to trigger an event dispatch.
        Event polling can take up to 5-6ms depending on how much is waiting.
        We prioritize the PeriodicCallers getting called on time, so we only
        dispatch events if there's a reasonable gap until the next needs to be
        called. */
    static constexpr double DISPATCH_MINIMUM_TIME_S = .003;

    /**
     * Dispatches waiting events to the eventHandlers.
     * Events are propagated through the vector in order, starting at index 0.
     * If an event is handled (handleOSEvent() returns true), propagation
     * stops.
     */
    void dispatchOSEvents();

    /**
     * Returns true if all PeriodicCallers have at least minimumTime left until
     * their next call. Else, false.
     */
    bool enoughTimeTillNextCall(double minimumTime);

    /**
     * Used to filter events before they arrive in the SDL event queue.
     *
     * Called asynchronously when SDL_PumpEvents is called (during
     * SDL_PollEvent). May run on a different core.
     */
    static int filterEvents(void* userData, SDL_Event* event);

    //-------------------------------------------------------------------------
    // SDL Objects
    //-------------------------------------------------------------------------
    SDL2pp::SDL sdl;

    /** Initializes UserConfig. Must be constructed after SDL is initialized
        and before anything else. */
    UserConfigInitializer userConfigInitializer;

    SDL2pp::Window sdlWindow;

    /** The SDL renderer that we use to render the UI and world.
        We use SDL2pp::Renderer for convenience of initialization here, but
        all other parts of the engine directly use SDL_Renderer (so that we
        can use SDL_Texture, which is better for the AssetCache.) */
    SDL2pp::Renderer sdlRenderer;

    //-------------------------------------------------------------------------
    // Modules, Dependencies, PeriodicCallers
    //-------------------------------------------------------------------------
    AssetCache assetCache;

    ResourceData resourceData;

    GraphicData graphicData;

    IconData iconData;

    Network network;
    /** Calls network.tick() at the network tick rate. */
    PeriodicCaller networkCaller;

    UserInterface userInterface;
    /** Calls userInterface.tick() at our UI tick rate. */
    PeriodicCaller uiCaller;

    Simulation simulation;
    /** Calls simulation.tick() at the sim tick rate. */
    PeriodicCaller simCaller;

    Renderer renderer;
    /** Calls renderer.render() at our frame rate. */
    PeriodicCaller rendererCaller;

    //-------------------------------------------------------------------------
    // Additional, used during the loop
    //-------------------------------------------------------------------------
    /** An ordered vector of OS-event-handling objects. */
    std::vector<OSEventHandler*> eventHandlers;

    /** True if there has been a request to exit the program, else false. */
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
void Application::registerRendererExtension()
{
    RendererExDependencies rendererDeps{
        sdlRenderer.Get(), simulation.getWorld(), userInterface,
        std::bind_front(&PeriodicCaller::getProgress, &simCaller)};

    renderer.setExtension(std::make_unique<T>(rendererDeps));
}

template<typename T>
void Application::registerSimulationExtension()
{
    SimulationExDependencies simulationDeps{
        simulation, userInterface.getEventDispatcher(), network, graphicData};

    simulation.setExtension(std::make_unique<T>(simulationDeps));
}

template<typename T>
void Application::registerUserInterfaceExtension()
{
    UserInterfaceExDependencies uiDeps{simulation,
                                       userInterface.getWorldObjectLocator(),
                                       userInterface.getEventDispatcher(),
                                       network,
                                       sdlRenderer.Get(),
                                       graphicData,
                                       iconData};

    userInterface.setExtension(std::make_unique<T>(uiDeps));
}

} // End namespace Client
} // End namespace AM
