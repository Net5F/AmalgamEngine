#pragma once

#include "Network.h"
#include "Simulation.h"
#include "Renderer.h"
#include "PeriodicCaller.h"

#include "SDL2pp/SDL.hh"
#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"

namespace AM
{
namespace Client
{

/**
 *
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
    /** We delay for 1ms when possible to reduce our CPU usage. We can't trust the
       scheduler to come back to us after exactly 1ms though, so we need to give it
       some leeway. Picked .003 = 3ms as a reasonable number. Open for tweaking. */
    static constexpr double DELAY_MINIMUM_TIME_S = .003;

    SDL2pp::SDL sdl;
    SDL2pp::Window sdlWindow;
    SDL2pp::Renderer sdlRenderer;

    Network network;
    PeriodicCaller networkCaller;

    Simulation sim;
    PeriodicCaller simCaller;

    Renderer renderer;
    PeriodicCaller rendererCaller;
};

} // End namespace Client
} // End namespace AM
