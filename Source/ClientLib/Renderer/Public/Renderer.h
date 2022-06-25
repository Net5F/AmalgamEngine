#pragma once

#include "OSEventHandler.h"
#include "PeriodicCaller.h"
#include "WorldSpritePreparer.h"

struct SDL_Renderer;

namespace AM
{
struct Sprite;
struct BoundingBox;
struct Camera;
struct ScreenRect;
namespace Client
{
class World;
class UserInterface;
class RendererHooks;

/**
 * Uses world information from the Sim to isometrically render the player's
 * view.
 */
class Renderer : public OSEventHandler
{
public:
    static constexpr unsigned int FRAMES_PER_SECOND = 60;
    static constexpr double FRAME_TIMESTEP_S
        = 1.0 / static_cast<double>(FRAMES_PER_SECOND);

    /**
     * @param getProgress  A function that returns how far between sim ticks we
     *                     are in decimal percent.
     */
    Renderer(SDL_Renderer* inSdlRenderer, World& inWorld, UserInterface& inUI,
             std::function<double(void)> inGetProgress);

    /**
     * First renders all tiles in view, then renders all entities in view.
     */
    void render();

    /**
     * Handles window events.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /**
     * See rendererHooks member comment.
     */
    void setRendererHooks(std::unique_ptr<RendererHooks> inRendererHooks);

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
     * Draws the given box. Useful for debug visuals.
     * Note: Don't use this for anything real, it's super slow.
     */
    void drawBoundingBox(const BoundingBox& box, const Camera& camera);

    SDL_Renderer* sdlRenderer;

    /** Used to grab the entity data that we need to render. */
    World& world;

    /** Used to begin the UI rendering. */
    UserInterface& ui;

    std::function<double(void)> getProgress;

    WorldSpritePreparer worldSpritePreparer;

    /** If non-nullptr, contains the project's rendering extension functions.
        Allows the project to provide rendering code and have it be called at 
        the appropriate time. */
    std::unique_ptr<RendererHooks> rendererHooks;
};

} // namespace Client
} // namespace AM
