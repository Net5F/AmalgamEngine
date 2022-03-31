#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"

namespace AM
{
class AssetCache;

namespace Client
{
class MainScreen;
class SpriteData;
class Sprite;
class BuildOverlay;

/**
 * The build panel on the main screen. Allows the user to select which tile
 * they want to add to the world.
 *
 * This panel is opened alongside BuildOverlay when we enter build mode.
 */
class BuildPanel : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    BuildPanel(AssetCache& inAssetCache, MainScreen& inScreen
               , SpriteData& inSpriteData, BuildOverlay& inBuildOverlay);

private:
    /**
     * Adds a tile thumbnail to the tileContainer.
     */
    void addTile(const Sprite& sprite);

    AssetCache& assetCache;

    /** Used to tell the build overlay which sprite to display. */
    BuildOverlay& buildOverlay;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer tileContainer;

    // TODO: Add buttons and a label for layer selection
    //       Cap it to MAX_TILE_LAYERS
};

} // End namespace Client
} // End namespace AM
