#pragma once

#include "EventHandler.h"
#include "PeriodicCaller.h"
#include "ScreenPoint.h"

#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"

namespace AM
{
class Sprite;
namespace Client
{
class Simulation;
class World;
class Camera;
class ScreenRect;

/**
 * Uses world information from the Sim to isometrically render the player's
 * view.
 */
class Renderer : public EventHandler
{
public:
    static constexpr unsigned int RENDER_FRAMES_PER_SECOND = 60;
    static constexpr double RENDER_FRAME_TIMESTEP_S
        = 1.0 / static_cast<double>(RENDER_FRAMES_PER_SECOND);

    /**
     * @param getProgress  A function that returns how far between sim ticks we
     *                     are in decimal percent.
     */
    Renderer(SDL2pp::Renderer& inSdlRenderer, SDL2pp::Window& inWindow,
             Simulation& inSim, std::function<double(void)> inGetProgress);

    /**
     * First renders all tiles in view, then renders all entities in view.
     */
    void render();

    /**
     * Handles window events.
     */
    bool handleEvent(SDL_Event& event) override;

private:
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
    ScreenPoint tileToScreen(int xIndex, int yIndex, float zoom);

    /**
     * Converts a point in the world to a point in screen space.
     * @return The screen space point that corresponds to the given point.
     */
    ScreenPoint worldToScreen(float x, float y, float zoom);

    /**
     * Returns true if the given extent is within the given camera's bounds,
     * else false.
     */
    bool isWithinCameraBounds(float x, float y, float width, float height,
                              Camera& camera);

    SDL2pp::Renderer& sdlRenderer;
    Simulation& sim;
    World& world;
    std::function<double(void)> getProgress;
};

} // namespace Client
} // namespace AM
