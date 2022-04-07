#pragma once

#include "MainButton.h"
#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"
#include "AUI/Text.h"

namespace AM
{
class AssetCache;

namespace Client
{
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
    BuildPanel(AssetCache& inAssetCache, SpriteData& inSpriteData,
               BuildOverlay& inBuildOverlay);

private:
    /**
     * Adds the eraser thumbnail to the tileContainer.
     * Used for clearing tile sprite layers.
     */
    void addEraser();

    /**
     * Adds a tile thumbnail to the tileContainer.
     */
    void addTile(const Sprite& sprite);

    /** Used to load textures for new thumbnails. */
    AssetCache& assetCache;

    /** Used to get the empty sprite when adding the eraser thumbnail. */
    SpriteData& spriteData;

    /** Used to tell the build overlay which sprite to display. */
    BuildOverlay& buildOverlay;

    /** The currently selected tile layer index. */
    unsigned int tileLayerIndex;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer tileContainer;

    AUI::Text layerLabel;

    MainButton layerDownButton;

    MainButton layerUpButton;
};

} // End namespace Client
} // End namespace AM
