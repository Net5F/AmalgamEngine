#pragma once

#include "EventHandler.h"
#include "PeriodicCaller.h"

#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"
#include "SDL2pp/Rect.hh"

namespace AM
{
class Sprite;
namespace Client
{
class Simulation;
class World;
class UserInterface;
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
             Simulation& inSim, UserInterface& inUI, std::function<double(void)> inGetProgress);

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
     * Draws the tiles from the World's tile map.
     * @param camera  The camera to draw with.
     */
    void drawTiles(const Camera& camera);

    /**
     * Draws all entities that have Sprite, Position and PreviousPosition
     * components.
     * @param camera  The camera to draw with.
     * @param alpha  The alpha to lerp between positions with.
     */
    void drawEntities(const Camera& camera, const double alpha);

    /**
     * Draws all elements of the UserInterface.
     * @param camera  The camera to draw with.
     */
    void drawUserInterface(const Camera& camera);

    /**
     * Returns true if the given extent is within the given camera's bounds,
     * else false.
     *
     * @param extent  An extent in final screen coordinates.
     * @param camera  A camera to use for screen width/height checks.
     */
    bool isWithinScreenBounds(const SDL2pp::Rect& extent, const Camera& camera);

    SDL2pp::Renderer& sdlRenderer;
    Simulation& sim;
    World& world;
    UserInterface& ui;
    std::function<double(void)> getProgress;
};

} // namespace Client
} // namespace AM
