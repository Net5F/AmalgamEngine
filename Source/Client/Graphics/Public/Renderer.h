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
class Camera;

/**
 * Uses world information from the Sim to isometrically render the player's
 * view.
 */
class Renderer
{
public:
    static constexpr unsigned int RENDER_TICKS_PER_SECOND = 60;
    static constexpr double RENDER_TICK_TIMESTEP_S
        = 1.0 / static_cast<double>(RENDER_TICKS_PER_SECOND);

    /** An unreasonable amount of time for the render tick to be late by.
        Late render ticks cause jittering, as the pacing between ticks becomes
        inconsistent. */
    static constexpr double RENDER_DELAYED_TIME_S = .001;

    Renderer(SDL2pp::Renderer& inSdlRenderer, Sim& inSim,
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
    /** Convenience struct for representing a point in screen space. */
    struct ScreenPoint {
        int x;
        int y;
    };

    /**
     * First renders all tiles in view, then renders all entities in view.
     */
    void render();

    /**
     * Renders the tiles from the World's tile map.
     * @param camera  The camera to render with.
     */
    void renderTiles(Camera& camera);

    /**
     * Renders all entities that have Sprite, Position and PreviousPosition
     * components.
     * @param camera  The camera to render with.
     * @param alpha  The alpha to lerp between positions with.
     */
    void renderEntities(Camera& camera, double alpha);

    /**
     * Converts a point from world space to screen space.
     * @return The screen space point that corresponds to worldPos.
     */
    ScreenPoint tileToScreen(int x, int y);

    SDL2pp::Renderer& sdlRenderer;
    Sim& sim;
    World& world;

    /** Used to time when we should render a frame. */
    Timer frameTimer;

    /** The aggregated time since we last processed a tick. */
    double accumulatedTime;
};

} // namespace Client
} // namespace AM
