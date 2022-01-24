#pragma once

#include "SpriteSheet.h"
#include "Sprite.h"
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
 * The left-hand panel on the main screen. Allows the user to manage the
 * project's sprite sheets.
 */
class SpritePanel : public AUI::Widget
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
    void addSprite(const SpriteSheet& sheet, Sprite& sprite);

    /**
     * If a sprite is active, refreshes that sprite's UI to match its
     * underlying data.
     */
    void refreshActiveSprite(const std::string& newDisplayName);

    /**
     * Clears spritesheetContainer, removing all the sprite sheet widgets.
     */
    void clearSprites();

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& parentOffset = {}) override;

private:
    /** Used to load the active sprite's texture. */
    AssetCache& assetCache;

    /** Used to save/clear the active sprite when a sprite thumbnail is
        activated or deactivated. */
    MainScreen& mainScreen;

    /** Used to get the current working dir when displaying the thumbnails. */
    SpriteDataModel& spriteDataModel;

    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spriteContainer;
};

} // End namespace SpriteEditor
} // End namespace AM
