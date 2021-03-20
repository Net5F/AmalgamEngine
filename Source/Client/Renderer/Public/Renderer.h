#pragma once

#include "EventHandler.h"
#include "PeriodicCaller.h"
#include "WorldSpritePreparer.h"

#include "SDL2pp/Window.hh"
#include "SDL2pp/Renderer.hh"
#include "SDL2pp/Rect.hh"

namespace AM
{
class Sprite;
class BoundingBox;
class Camera;
class ScreenRect;
namespace Client
{
class Simulation;
class World;
class UserInterface;

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
             Simulation& inSim, UserInterface& inUI,
             std::function<double(void)> inGetProgress);

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
     * Renders the vector of sprites returned by
     * worldSpritePreparer.prepareSprites().
     *
     * @param camera  The camera to calculate screen position with.
     * @param alpha  For entities, the alpha to lerp between positions with.
     */
    void renderWorld(const Camera& camera, double alpha);

    /**
     * Render all elements of the UserInterface.
     * @param camera  The camera to draw with.
     */
    void renderUserInterface(const Camera& camera);

    /**
     * Draws the given box. Useful for debug visuals.
     * Note: Don't use this for anything real, it's super slow.
     */
    void drawBoundingBox(const BoundingBox& box, const Camera& camera);

    SDL2pp::Renderer& sdlRenderer;
    Simulation& sim;
    World& world;
    UserInterface& ui;
    std::function<double(void)> getProgress;

    WorldSpritePreparer worldSpritePreparer;
};

} // namespace Client
} // namespace AM
