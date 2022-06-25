#pragma once

#include "OSEventHandler.h"
#include <memory>

struct SDL_Renderer;

namespace AM
{
struct Camera;

namespace Client
{
class World;

/**
 * Defines an extension for the engine's UserInterface class.
 *
 * Extensions are implemented by the project, and are given generic functions 
 * ("hooks") in which they can implement relevant project logic.
 *
 * The project can register the extension class with the engine through 
 * Application::registerUserInterfaceExtension().
 */
class IUserInterfaceExtension : public OSEventHandler
{
public:
    // Canonical constructor (derived class must implement):
    // UserInterfaceExtension(UserInterfaceExDependencies deps)

    /**
     * Called during the UI tick.
     */
    virtual void tick(double timestepS) = 0;

    /**
     * Called when the UI should be rendered.
     */
    virtual void render(const Camera& camera) = 0;

    /**
     * See OSEventHandler for details.
     */
    bool handleOSEvent(SDL_Event& event) override = 0;
};

} // namespace Client
} // namespace AM
