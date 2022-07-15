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
class IRendererExtension;

/**
 * Uses world information from the Sim to isometrically render the player's
 * view.
 */
class Renderer : public OSEventHandler
{
public:
    /**
     * @param getSimTickProgress  A function that returns how far between sim
     *                            ticks we are in decimal percent.
     */
    Renderer(SDL_Renderer* inSdlRenderer, World& inWorld, UserInterface& inUI,
             std::function<double(void)> inGetSimTickProgress);

    /**
     * First renders all tiles in view, then renders all entities in view.
     */
    void render();

    /**
     * Handles window events.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<IRendererExtension> inExtension);

private:
    /**
     * Renders the vector of sprites returned by
     * worldSpritePreparer.prepareSprites().
     *
     * @param camera  The camera to calculate screen position with.
     * @param alpha  How far we are between sim ticks, as a percentage from
     *               0.0 to 1.0. Used to interpolate between positions.
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

    std::function<double(void)> getSimTickProgress;

    WorldSpritePreparer worldSpritePreparer;

    /** If non-nullptr, contains the project's rendering extension functions.
        Allows the project to provide rendering code and have it be called at
        the appropriate time. */
    std::unique_ptr<IRendererExtension> extension;
};

} // namespace Client
} // namespace AM
