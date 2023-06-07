#pragma once

#include "OSEventHandler.h"
#include "TileLayers.h"
#include "PhantomTileSpriteInfo.h"
#include "TileSpriteColorModInfo.h"
#include "QueuedEvents.h"
#include <memory>

namespace AM
{
struct Camera;

namespace Client
{

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
     * Returns all of the project UI's phantom tile sprites.
     * Phantom tile sprites are used when you want to visually add or replace a
     * tile layer in the sim's tile map without actually modifying the map.
     */
    std::vector<PhantomTileSpriteInfo> getPhantomTileSprites() const;

    /**
     * Returns all of the project UI's tile sprite color modifications.
     * Color mods are used when you want to modify the color or transparency 
     * of a tile layer in the sim's tile map.
     */
    std::vector<TileSpriteColorModInfo> getTileSpriteColorMods() const;

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
     */
    void render(const Camera& camera);

    /**
     * Passes the given event to the project's UI handler, if present.
     */
    bool handleOSEvent(SDL_Event& event) override;

    /**
     * Returns the UI event dispatcher. Used to send events to the simulation.
     */
    EventDispatcher& getEventDispatcher();

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<IUserInterfaceExtension> inExtension);

private:
    /** Used to dispatch events from the UI to the simulation. */
    EventDispatcher eventDispatcher;

    /** If non-nullptr, contains the project's UI extension functions.
        Allows the project to provide UI code and have it be called at the
        appropriate time. */
    std::unique_ptr<IUserInterfaceExtension> extension;

    /** Holds any phantom tile sprites that the UI wants rendered. */
    std::vector<PhantomTileSpriteInfo> phantomTileSprites;

    /** Holds any color mods that the UI wants applied when rendering. */
    std::vector<TileSpriteColorModInfo> tileSpriteColorMods;
};

} // End namespace Client
} // End namespace AM
