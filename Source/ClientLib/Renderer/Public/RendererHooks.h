#pragma once

#include "OSEventHandler.h"

struct SDL_Renderer;

namespace AM
{
struct Camera;
namespace Client
{
class World;

/**
 * Provides hooks that the project can use to have code called during 
 * rendering.
 * 
 * Also provides any dependencies that the project's rendering logic may need.
 */
class RendererHooks : public OSEventHandler
{
public:
    /**
     * Called before tiles and entities are rendered.
     */
    virtual void beforeWorld(const Camera& lerpedCamera, double alpha) = 0;

    /**
     * Called after tiles and entities are rendered.
     */
    virtual void afterWorld(const Camera& lerpedCamera, double alpha) = 0;

    /**
     * See OSEventHandler for details.
     * 
     * Note: Renderer will pass events to this class first. If the event is 
     *       not handled, then Renderer will attempt to handle it.
     */
    bool handleOSEvent(SDL_Event& event) override = 0;
    
    //-------------------------------------------------------------------------
    // Setters
    //-------------------------------------------------------------------------
    // Note: These are called by Application::registerRendererExtension().
    void setSdlRenderer(SDL_Renderer* inSdlRenderer);
    void setWorld(const World* inWorld);

private:
    /** Used for making SDL2 render calls. */
    SDL_Renderer* sdlRenderer;

    /** Used for observing world state to render. */
    const World* world;
};

} // namespace Client
} // namespace AM
