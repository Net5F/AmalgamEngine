#pragma once

#include "LibraryItemData.h"
#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Text.h"
#include "AUI/ScrollArea.h"

namespace AUI
{
class Image;
}

namespace AM
{
namespace ResourceImporter
{
class DataModel;

/**
 * The center stage shown when the user loads a sprite from the Library.
 * Allows the user to edit the active sprite's bounding box.
 */
class SpriteSheetEditView : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteSheetEditView(DataModel& inDataModel);

private:
    /**
     * If the new active item is a sprite sheet, loads it's data onto this stage.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * Adds the sprite to the active sheet and refreshes the texture.
     */
    void onSpriteAdded(SpriteID spriteID);

    /**
     * Removes the sprite from the active sheet and refreshes the texture.
     * (If active sprite was removed) Sets activeSprite to invalid and returns
     * the stage to its default state.
     */
    void onSpriteRemoved(SpriteID spriteID);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /**
     * Generates a sprite sheet texture containing all of the sheet's sprites, 
     * placed at their current textureExtents.
     */
    SDL_Texture*
        generateSpriteSheetTexture(const EditorSpriteSheet& spriteSheet);

    /** Used to get the current working dir when displaying the sprite sheet. */
    DataModel& dataModel;

    /** The active sprite sheet's ID. */
    SpriteSheetID activeSpriteSheetID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;

    /** Holds the sprite sheet image so it can be scrolled. */
    AUI::ScrollArea spriteSheetScrollArea;

    /** The sprite sheet that is currently loaded. */
    AUI::Image* spriteSheetImage;

    AUI::Text descText;
};

} // End namespace ResourceImporter
} // End namespace AM
