#pragma once

#include "OSEventHandler.h"
#include "PhantomSpriteInfo.h"
#include "SpriteColorModInfo.h"
#include <vector>

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
    // UserInterfaceExtension(const UserInterfaceExDependencies& deps)

    /**
     * Returns all of the project UI's phantom sprites.
     * Phantom sprites are used when you want to visually add or replace a tile
     * layer or entity without actually modifying the sim.
     */
    virtual std::vector<PhantomSpriteInfo> getPhantomSprites() const = 0;

    /**
     * Returns all of the project UI's sprite color modifications.
     * Color mods are used when you want to modify the color or transparency
     * of a tile layer or entity.
     */
    virtual std::vector<SpriteColorModInfo> getSpriteColorMods() const = 0;

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
