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
#include "ItemData.h"
#include "CastableData.h"
#include "QueuedEvents.h"
#include "UserInterface.h"
#include "PeriodicCaller.h"
#include "MessageProcessorContext.h"
#include "RendererContext.h"
#include "SimulationContext.h"
#include "UserInterfaceContext.h"
#include "IMessageProcessorExtension.h"
#include "IRendererExtension.h"
#include "ISimulationExtension.h"
#include "IUserInterfaceExtension.h"

#include "entt/signal/dispatcher.hpp"

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
     * 
     * Note: All extension classes must be registered before calling this 
     *       function.
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
    static constexpr double DISPATCH_MINIMUM_TIME_S{.003};

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
    // Event Busses
    //-------------------------------------------------------------------------
    /** Used for Sim -> UI events. */
    entt::dispatcher simEventDispatcher;

    /** Used for UI -> Sim events. */
    entt::dispatcher uiEventDispatcher;

    /** Used for Network -> Sim/UI message events. */
    EventDispatcher networkEventDispatcher;

    //-------------------------------------------------------------------------
    // Data, Modules, Contexts
    //-------------------------------------------------------------------------
    AssetCache assetCache;
    ResourceData resourceData;
    GraphicData graphicData;
    IconData iconData;
    ItemData itemData;
    CastableData castableData;

    MessageProcessorContext messageProcessorContext;
    Network network;

    SimulationContext simulationContext;
    Simulation simulation;

    UserInterfaceContext uiContext;
    UserInterface userInterface;

    RendererContext rendererContext;
    Renderer renderer;

    //-------------------------------------------------------------------------
    // PeriodicCallers
    //-------------------------------------------------------------------------
    /** Calls network.tick() at the network tick rate. */
    PeriodicCaller networkCaller;

    /** Calls userInterface.tick() at our UI tick rate. */
    PeriodicCaller uiCaller;

    /** Calls simulation.tick() at the sim tick rate. */
    PeriodicCaller simCaller;

    /** Calls renderer.render() at our frame rate. */
    PeriodicCaller rendererCaller;

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
    std::unique_ptr<IRendererExtension> rendererExtension;
    std::unique_ptr<ISimulationExtension> simulationExtension;
    std::unique_ptr<IUserInterfaceExtension> userInterfaceExtension;

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
    network.setMessageProcessorExtension(
        std::make_unique<T>(messageProcessorContext));
}

template<typename T>
void Application::registerRendererExtension()
{
    renderer.setExtension(std::make_unique<T>(rendererContext));
}

template<typename T>
void Application::registerSimulationExtension()
{
    simulation.setExtension(std::make_unique<T>(simulationContext));
}

template<typename T>
void Application::registerUserInterfaceExtension()
{
    userInterface.setExtension(std::make_unique<T>(uiContext));
}

} // End namespace Client
} // End namespace AM
