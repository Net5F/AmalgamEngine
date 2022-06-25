#pragma once

#include "OSEventHandler.h"
#include "Network.h"
#include "Simulation.h"
#include "Renderer.h"
#include "AssetCache.h"
#include "SpriteData.h"
#include "QueuedEvents.h"
#include "UserInterface.h"
#include "PeriodicCaller.h"
#include "IRendererExtension.h"
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
     * Registers an extension class to be called by the Renderer.
     * 
     * Note: The extension class type T must derive from IRendererExtension 
     *       and must implement a constructor of the form 
     *       T(RendererExDependencies).
     */
    template<typename T>
    void registerRendererExtension();

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
    /** Event polling can take up to 5-6ms depending on how much is waiting.
        We prioritize the PeriodicCallers getting called on time, so we only
        dispatch events if there's a reasonable gap until the next needs to be
        called. */
    static constexpr double DISPATCH_MINIMUM_TIME_S = .003;

    /** We sleep for 1ms when possible to reduce our CPU usage. We can't trust
        the scheduler to come back to us after exactly 1ms though, so we need
        to give it some leeway. Picked .003 = 3ms as a reasonable number. Open
        for tweaking. */
    static constexpr double SLEEP_MINIMUM_TIME_S = .003;

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

    SpriteData spriteData;

    /** Used to dispatch events from the UI to the simulation.
        Owned by the Application to break the sim's dependency on the UI. */
    EventDispatcher uiEventDispatcher;

    Network network;
    /** Calls network.tick() at the network tick rate. */
    PeriodicCaller networkCaller;

    Simulation sim;
    /** Calls sim.tick() at the sim tick rate. */
    PeriodicCaller simCaller;

    UserInterface userInterface;
    /** Calls userInterface.tick() at our UI tick rate. */
    PeriodicCaller uiCaller;

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
void Application::registerRendererExtension()
{
    UserInterfaceExDependencies rendererDeps{
        sdlRenderer.Get(), sim.getWorld(), userInterface,
        std::bind_front(&PeriodicCaller::getProgress, &simCaller)};

    userInterface.setExtension(std::make_unique<T>(rendererDeps));
}

template<typename T>
void Application::registerUserInterfaceExtension()
{
    UserInterfaceExDependencies uiDeps{sim.getWorld().worldSignals,
                                       uiEventDispatcher, sdlRenderer.Get(),
                                       assetCache, spriteData};

    userInterface.setExtension(std::make_unique<T>(uiDeps));
}

} // End namespace Client
} // End namespace AM
