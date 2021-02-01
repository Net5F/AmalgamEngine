#pragma once

#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"
#include "Timer.h"

namespace AM
{
class Sprite;
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
     * Converts a tile's x,y indices into screen space coordinates.
     * @return The screen space point that corresponds to the given indices.
     */
    SDL2pp::Point tileToScreen(int xIndex, int yIndex);

    /**
     * Converts a sprite's world position to a point in screen space.
     * @return The screen space point that corresponds to the given point.
     */
    SDL2pp::Point spriteToScreen(float x, float y);

    /**
     * Returns true if the given sprite at the given screen position is within
     * the given camera's bounds, else false.
     */
    bool isWithinCameraBounds(int x, int y, Sprite& sprite, Camera& camera);

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
