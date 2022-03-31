#pragma once

#include "AUI/Screen.h"
#include "Camera.h"
#include "BuildPanel.h"
#include "BuildOverlay.h"
#include "TileExtent.h"

namespace AM
{
class EventDispatcher;
class AssetCache;

namespace Client
{
class SpriteData;

/**
 * The main UI that overlays the world.
 */
class MainScreen : public AUI::Screen
{
public:
    MainScreen(EventDispatcher& inUiEventDispatcher, AssetCache& inAssetCache
               , SpriteData& inSpriteData);

    /**
     * Sets the camera to use when rendering.
     *
     * Called by the renderer to give us the lerped camera for the current
     * frame.
     */
    void setCamera(const Camera& inCamera);

    /**
     * Sets the size of the world map in any widgets that care.
     *
     * Used in build mode to make sure we aren't rendering or requesting
     * changes to tiles that are out of bounds.
     */
    void setTileMapExtent(TileExtent inTileExtent);

    void render() override;

private:
    //-------------------------------------------------------------------------
    // Windows
    //-------------------------------------------------------------------------
    /** The build mode overlay. Allows the user to place tiles. */
    BuildOverlay buildOverlay;

    /** The build mode panel. Allows the user to select tiles. */
    BuildPanel buildPanel;
};

} // End namespace Client
} // End namespace AM
