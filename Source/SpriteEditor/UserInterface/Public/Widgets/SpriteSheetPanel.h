#pragma once

#include "SpriteSheet.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"
#include "AUI/Button.h"
#include "AddSheetDialog.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
class MainScreen;
class SpriteDataModel;

/**
 * The left-side panel on the main screen. Allows the user to manage the
 * project's sprite sheets.
 */
class SpriteSheetPanel : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteSheetPanel(AssetCache& inAssetCache, MainScreen& inScreen,
                     SpriteDataModel& inSpriteDataModel);

    /**
     * Adds a MainThumbnail widget to the spritesheetContainer, using the
     * given data.
     */
    void addSpriteSheet(const SpriteSheet& sheet);

    /**
     * Clears spritesheetContainer, removing all the sprite sheet widgets.
     */
    void clearSpriteSheets();

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    bool onMouseButtonDown(SDL_MouseButtonEvent& event) override;

private:
    /** Used to load the added sprite sheet's textures. */
    AssetCache& assetCache;

    /** Used to open the confirmation dialog when removing a sheet. */
    MainScreen& mainScreen;

    /** Used to update the model when a sheet is removed. */
    SpriteDataModel& spriteDataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spriteSheetContainer;

    AUI::Button remSheetButton;

    AUI::Button addSheetButton;

    AddSheetDialog addSheetDialog;
};

} // End namespace SpriteEditor
} // End namespace AM
