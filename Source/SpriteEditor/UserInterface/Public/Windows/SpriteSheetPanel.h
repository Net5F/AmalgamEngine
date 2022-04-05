#pragma once

#include "SpriteSheet.h"
#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"
#include "AUI/Button.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
class MainScreen;
class SpriteDataModel;
class MainThumbnail;

// TODO: Make this obtain focus and deselect all selected thumbnails when
//       focus is lost.
/**
 * The left-side panel on the main screen. Allows the user to manage the
 * project's sprite sheets.
 */
class SpriteSheetPanel : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteSheetPanel(AssetCache& inAssetCache, MainScreen& inScreen,
                     SpriteDataModel& inSpriteDataModel);

private:
    /**
     * Adds a new MainThumbnail for the given sheet to the
     * spriteSheetContainer.
     */
    void onSheetAdded(unsigned int sheetID, const SpriteSheet& sheet);

    /**
     * Removes the thumbnail associated with the given sheet.
     */
    void onSheetRemoved(unsigned int sheetID);

    /** Used to load the added sprite sheet's textures. */
    AssetCache& assetCache;

    /** Used to open the confirmation dialog when removing a sheet. */
    MainScreen& mainScreen;

    /** Used to update the model when a sheet is removed. */
    SpriteDataModel& spriteDataModel;

    /** Maps sprite sheet IDs to their associated thumbnails. */
    std::unordered_map<unsigned int, MainThumbnail*> thumbnailMap;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spriteSheetContainer;

    AUI::Button remSheetButton;

    AUI::Button addSheetButton;
};

} // End namespace SpriteEditor
} // End namespace AM
