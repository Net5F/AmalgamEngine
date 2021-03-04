#pragma once

#include "EventHandler.h"
#include "PeriodicCaller.h"

#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"

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
     * Renders all elements of the UserInterface.
     * @param camera  The camera to render with.
     */
    void renderUserInterface(Camera& camera);

    /**
     * Returns true if the given extent is within the given camera's bounds,
     * else false.
     */
    bool isWithinCameraBounds(float x, float y, float width, float height,
                              Camera& camera);

    SDL2pp::Renderer& sdlRenderer;
    Simulation& sim;
    World& world;
    UserInterface& ui;
    std::function<double(void)> getProgress;
};

} // namespace Client
} // namespace AM
