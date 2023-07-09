#pragma once

#include "OSEventHandler.h"
#include "TileLayers.h"
#include "PhantomSpriteInfo.h"
#include "SpriteColorModInfo.h"
#include "SpriteSortInfo.h"
#include "WorldObjectLocator.h"
#include "QueuedEvents.h"
#include <memory>

namespace AM
{
struct Camera;

namespace Client
{
class World;
class IUserInterfaceExtension;

/**
 * Drives the project's UI through the extension interface.
 *
 * Provides a way for the Renderer to know how the UI wants to affect the world
 * rendering (which sprites to highlight, phantom sprites to insert for build 
 * mode, etc).
 */
class UserInterface : public OSEventHandler
{
public:
    UserInterface();

    /**
     * Returns all of the project UI's phantom sprites.
     * Phantom sprites are used when you want to visually add or replace a tile 
     * layer or entity without actually modifying the sim.
     */
    std::vector<PhantomSpriteInfo> getPhantomSprites() const;

    /**
     * Returns all of the project UI's sprite color modifications.
     * Color mods are used when you want to modify the color or transparency 
     * of a tile layer or entity.
     */
    std::vector<SpriteColorModInfo> getSpriteColorMods() const;

    /**
     * Calls the project's UI tick(), if present.
     *
     * @param tick  The time since this function was last called.
     */
    void tick(double timestepS);

    /**
     * Calls the project's UI render(), if present.
     *
     * @param camera  The camera to calculate screen position with.
     * @param sortedSprites  The sorted list of world sprites that were drawn on 
     *                       this frame. Used to update worldObjectLocator.
     */
    void render(const Camera& camera,
                const std::vector<SpriteSortInfo>& sortedSprites);

    /**
     * Passes the given event to the project's UI handler, if present.
     */
    bool handleOSEvent(SDL_Event& event) override;

    const WorldObjectLocator& getWorldObjectLocator();

    /**
     * Returns the UI event dispatcher. Used to send events to the simulation.
     */
    EventDispatcher& getEventDispatcher();

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<IUserInterfaceExtension> inExtension);

    /**
     * Separate setter to avoid a circular dependency during construction 
     * (Simulation needs our eventDispatcher, we need Simulation's World).
     */
    void setWorld(const World& inWorld);

private:
    /** Used to get tile map info for the locator. */
    const World* world;

    /** Used to dispatch events from the UI to the simulation. */
    EventDispatcher eventDispatcher;

    /** If non-nullptr, contains the project's UI extension functions.
        Allows the project to provide UI code and have it be called at the
        appropriate time. */
    std::unique_ptr<IUserInterfaceExtension> extension;

    /** Used in hit testing for mouse events. */
    WorldObjectLocator worldObjectLocator;
};

} // End namespace Client
} // End namespace AM
