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
