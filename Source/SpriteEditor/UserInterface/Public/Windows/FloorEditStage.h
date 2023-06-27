#pragma once

#include "LibraryItemData.h"
#include "LibraryListItem.h"
#include "SpriteSets.h"
#include "SpriteSetSlot.h"
#include "AUI/Window.h"
#include "AUI/Text.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"

namespace AM
{
namespace SpriteEditor
{
class SpriteDataModel;
class LibraryWindow;

/**
 * The center stage shown when the user loads a floor from the Library.
 * Allows the user to edit the active floor's sprites.
 */
class FloorEditStage : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    FloorEditStage(SpriteDataModel& inSpriteDataModel,
                   const LibraryWindow& inLibraryWindow);

private:
    /**
     * If the new active item is a floor, loads it's data onto this stage.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If type/ID matches active set) Sets this stage back to its default state.
     */
    void onSpriteSetRemoved(SpriteSet::Type type, Uint16 spriteSetID);

    /**
     * (If type/ID matches active set) Sets the given index to the given sprite.
     */
    void onSpriteSetSlotChanged(SpriteSet::Type type, Uint16 spriteSetID,
                                    std::size_t index, int newSpriteID);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /**
     * Returns the appropriate top text for the given index.
     */
    std::string getTopText(std::size_t spriteSetIndex);

    /**
     * Fills the given slot widget with the given sprite's image and name.
     */
    void fillSlotSpriteData(SpriteSetSlot& slot, int spriteID);

    /** Used to get the current working dir when displaying the sprite. */
    SpriteDataModel& spriteDataModel;

    /** Used to get the currently selected list item. */
    const LibraryWindow& libraryWindow;

    /** The active floor's ID. */
    Uint16 activeFloorID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;
    AUI::Text modifyText;
    AUI::Text clearText;

    /** Holds this sprite set's sprites. */
    AUI::VerticalGridContainer spriteContainer;

    AUI::Text descText1;
    AUI::Text descText2;
    AUI::Text descText3;
};

} // End namespace SpriteEditor
} // End namespace AM
