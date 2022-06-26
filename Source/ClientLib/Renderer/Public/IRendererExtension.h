#pragma once

#include "OSEventHandler.h"

namespace AM
{
struct Camera;
namespace Client
{

/**
 * Defines an extension for the engine's Client::Renderer class.
 *
 * Extensions are implemented by the project, and are given generic functions 
 * ("hooks") in which they can implement relevant project logic.
 *
 * The project can register the extension class with the engine through 
 * Application::registerRendererExtension().
 */
class IRendererExtension : public OSEventHandler
{
public:
    // Canonical constructor (derived class must implement):
    // RendererExtension(RendererExDependencies deps)

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
};

} // namespace Client
} // namespace AM
