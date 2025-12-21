#pragma once

#include "OSEventHandler.h"
#include "PeriodicCaller.h"
#include "WorldSpriteSorter.h"

struct SDL_Renderer;

namespace AM
{
struct Sprite;
struct BoundingBox;
struct Camera;
namespace Client
{
struct RendererContext;
class World;
class UserInterface;
class GraphicData;
class IRendererExtension;

/**
 * Pulls information from the Simulation and other sources to isometrically
 * render the player's view of the world.
 *
 * Also kicks off the UI rendering.
 */
class Renderer : public OSEventHandler
{
public:
    Renderer(const RendererContext& inRendererContext);

    /**
     * Renders the player's view of the world, then kicks off the UI rendering.
     */
    void render();

    /**
     * Handles window events.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /**
     * See extension member comment.
     */
    void setExtension(IRendererExtension* inExtension);

private:
    /**
     * Gets the player entity's current camera, lerped to match the given alpha.
     */
    Camera getLerpedCamera(double alpha);

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
     * Renders a sprite using the given info.
     */
    void renderSprite(const SpriteSortInfo& spriteInfo);

    /**
     * Renders the given entity's visual effects, if it has any.
     */
    void renderEntityVisualEffects(entt::entity entity);

    /**
     * Draws the given box. Useful for debug visuals.
     * Note: Don't use this for anything real, it's super slow.
     */
    void drawBoundingBox(const BoundingBox& box, const Camera& camera);

    SDL_Renderer* sdlRenderer;

    /** Used to grab the entity data that we need to render. */
    World& world;

    /** Used to begin the UI rendering. */
    UserInterface& userInterface;

    /** Used for getting graphics render data. */
    GraphicData& graphicData;

    std::function<double(void)> getSimTickProgress;

    WorldSpriteSorter worldSpriteSorter;

    /** Contains the project's rendering extension functions.
        Allows the project to provide rendering code and have it be called at 
        the appropriate time. */
    IRendererExtension* extension;
};

} // namespace Client
} // namespace AM
