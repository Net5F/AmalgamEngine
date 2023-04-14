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
class MainCollapsibleContainer;
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

// TODO: Make this obtain focus and deselect all selected thumbnails when
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
     * Adds the given sprite to the given sprite sheet widget.
     */
    void addSpriteToSheetWidget(
        MainCollapsibleContainer& sheetWidget,
        const SpriteSheet& sheet, unsigned int spriteID);

    /**
     * Deactivates any activated list items in any library category, except 
     * for the given list item.
     */
    void deactivateListItems(const LibraryListItem* newActiveListItem);

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

    AUI::VerticalListContainer categoryContainer;

    AUI::Button newButton;
};

} // End namespace SpriteEditor
} // End namespace AM
