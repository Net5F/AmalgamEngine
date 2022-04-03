#pragma once

#include "SpriteSheet.h"
#include "Sprite.h"
#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
class MainScreen;
class SpriteDataModel;

/**
 * The bottom panel on the main screen. Allows the user to select sprites to
 * load onto the stage.
 */
class SpritePanel : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpritePanel(AssetCache& inAssetCache, MainScreen& inScreen,
                SpriteDataModel& inSpriteDataModel);

    /**
     * Adds a MainThumbnail widget to the spriteContainer, using the
     * given data.
     */
    void addSprite(const Sprite& sprite);

    /**
     * If a sprite is active, refreshes that sprite's UI to match its
     * underlying data.
     */
    void refreshActiveSprite(const std::string& newDisplayName);

    /**
     * Clears spritesheetContainer, removing all the sprite sheet widgets.
     */
    void clearSprites();

private:
    /** Used to load the active sprite's texture. */
    AssetCache& assetCache;

    /** Used to save/clear the active sprite when a sprite thumbnail is
        activated or deactivated. */
    MainScreen& mainScreen;

    /** Used to get the current working dir when displaying the thumbnails. */
    SpriteDataModel& spriteDataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spriteContainer;
};

} // End namespace SpriteEditor
} // End namespace AM
