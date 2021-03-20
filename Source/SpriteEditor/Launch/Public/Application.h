#pragma once

#include "EventHandler.h"
#include "PeriodicCaller.h"
#include "ResourceManager.h"

#include "SDL2pp/SDL.hh"
#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"

#include <atomic>

namespace AM
{
namespace SpriteEditor
{
/**
 * Maintains the lifetime of all app modules (sim, network, etc) and manages
 * the main thread's loop.
 */
class Application : public EventHandler
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
    bool handleEvent(SDL_Event& event) override;

private:
    /**
     * Dispatches waiting events to the eventHandlers.
     * Events are propagated through the vector in order, starting at index 0.
     * If an event is handled (handleEvent() returns true), propagation stops.
     */
    void dispatchEvents();

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

    SDL2pp::SDL sdl;
    SDL2pp::Window sdlWindow;
    SDL2pp::Renderer sdlRenderer;

    ResourceManager resourceManager;

    /** An ordered vector of event handlers. */
    std::vector<EventHandler*> eventHandlers;

    /** True if there has been a request to exit the program, else false. */
    std::atomic<bool> exitRequested;
};

} // End namespace SpriteEditor
} // End namespace AM
