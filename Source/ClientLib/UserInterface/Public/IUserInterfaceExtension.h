#pragma once

#include "OSEventHandler.h"

namespace AM
{
struct Camera;

namespace Client
{

/**
 * Defines an extension for the engine's Client::UserInterface class.
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
     *
     * @param tick  The time since this function was last called.
     */
    virtual void tick(double timestepS) = 0;

    /**
     * Called when the UI should be rendered.
     *
     * @param camera  The camera to calculate screen position with.
     */
    virtual void render(const Camera& camera) = 0;

    /**
     * See OSEventHandler for details.
     */
    bool handleOSEvent(SDL_Event& event) override = 0;
};

} // namespace Client
} // namespace AM
