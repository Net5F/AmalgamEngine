#pragma once

#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"
#include "Timer.h"

namespace AM
{
namespace Client
{
class Sim;
class World;

class RenderSystem
{
public:
    static constexpr unsigned int RENDER_TICKS_PER_SECOND = 60;
    static constexpr double RENDER_TICK_TIMESTEP_S
        = 1.0 / static_cast<double>(RENDER_TICKS_PER_SECOND);

    /** An unreasonable amount of time for the render tick to be late by.
        Late render ticks cause jittering, as the pacing between ticks becomes
        inconsistent. */
    static constexpr double RENDER_DELAYED_TIME_S = .001;

    RenderSystem(SDL2pp::Renderer& inRenderer, Sim& inSim,
                 SDL2pp::Window& inWindow);

    void tick();

    /** Initialize the frame timer. */
    void initTimer();

    /**
     * Returns how far we are temporally into our wait for the next frame
     * render. e.g. .01 if we're 10% of the way to the next frame.
     */
    double getTimeTillNextFrame();

private:
    /**
     * Renders all entities with Sprite, PreviousPosition, and Position
     * components, using the given alpha to lerp their position.
     */
    void render(double alpha);

    SDL2pp::Renderer& renderer;
    Sim& sim;
    World& world;

    /** Used to time when we should render a frame. */
    Timer frameTimer;

    /** The aggregated time since we last processed a tick. */
    double accumulatedTime;
};

} // namespace Client
} // namespace AM
