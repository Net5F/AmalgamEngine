#pragma once

#include "SpriteSheet.h"
#include "AUI/Window.h"
#include "AUI/Text.h"
#include "AUI/Image.h"
#include "AUI/VerticalListContainer.h"
#include "AUI/Button.h"

namespace AM
{
namespace SpriteEditor
{
class MainScreen;
class SpriteDataModel;
class SpriteSheetListItem;
class LibraryListItem;

/**
 * The top-level categories that we have in the library.
 * These values are used to index into the categoryContainer.
 */
struct Category {
    enum Value
    {
        SpriteSheets,
        Count
    };
};

// TODO: Make this obtain focus and deselect all selected things when
//       focus is lost.
/**
 * The left-side panel on the main screen. Allows the user to manage the
 * project's sprite sheets.
 */
class LibraryWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    LibraryWindow(MainScreen& inScreen, SpriteDataModel& inSpriteDataModel);

private:
    /**
     * Adds the given sheet to the library.
     */
    void onSheetAdded(unsigned int sheetID, const SpriteSheet& sheet);

    /**
     * Removes the given sheet from the library.
     */
    void onSheetRemoved(unsigned int sheetID);

    /**
     * Adds the given sprite to the given sprite sheet list item.
     */
    void addSpriteToSheetListItem(
        SpriteSheetListItem& sheetListItem,
        const SpriteSheet& sheet, unsigned int spriteID);

    /**
     * Iterates all list items in the library, applying the given transform.
     */
    template<class UnaryOperation>
    void transformListItems(UnaryOperation unaryOp);

    /** Used to open the confirmation dialog when removing a sheet. */
    MainScreen& mainScreen;

    /** Used to update the model when a sheet is removed. */
    SpriteDataModel& spriteDataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;

    AUI::VerticalListContainer libraryContainer;

    AUI::Button newButton;
};

} // End namespace SpriteEditor
} // End namespace AM
