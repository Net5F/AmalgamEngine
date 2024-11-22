#pragma once

#include "AUI/Screen.h"
#include "AUI/Button.h"
#include "AUI/ConfirmationDialog.h"
#include "LibraryItemData.h"
#include "LibraryWindow.h"
#include "SpriteSheetEditView.h"
#include "SpriteSheetPropertiesWindow.h"
#include "BoundingBoxEditView.h"
#include "BoundingBoxPropertiesWindow.h"
#include "SpriteEditView.h"
#include "SpritePropertiesWindow.h"
#include "AnimationElementsWindow.h"
#include "AnimationEditView.h"
#include "AnimationPropertiesWindow.h"
#include "GraphicSetEditView.h"
#include "GraphicSetPropertiesWindow.h"
#include "EntityGraphicSetEditView.h"
#include "EntityGraphicSetPropertiesWindow.h"
#include "IconEditView.h"
#include "IconPropertiesWindow.h"
#include "TitleButton.h"
#include "LibraryAddMenu.h"
#include "HamburgerButtonWindow.h"
#include "HamburgerMenu.h"
#include "AddIconSheetDialog.h"
#include "AddSpriteDialog.h"
#include "SaveBoundingBoxDialog.h"

namespace AM
{
namespace ResourceImporter
{
class DataModel;

/**
 * The main screen for doing work.
 */
class MainScreen : public AUI::Screen
{
public:
    MainScreen(DataModel& inDataModel);

    /**
     * Opens a confirmation dialog with a "Confirm" button and a "Cancel" 
     * button.
     *
     * @param bodyText The main dialog text.
     * @param confirmButtonText The text on the confirm button.
     * @param onConfirmation Called when the user presses the confirm button.
     */
    void openConfirmationDialog(const std::string& bodyText,
                                const std::string& confirmButtonText,
                                std::function<void(void)> onConfirmation);

    /**
     * Opens an error dialog with a "Cancel" button.
     *
     * @param bodyText The main dialog text.
     */
    void openErrorDialog(const std::string& bodyText);

    /**
     * Opens the dialog for saving a BoundingBox to the Library.
     */
    void openSaveBoundingBoxDialog(
        const BoundingBox& modelBoundsToSave,
        std::function<void(BoundingBoxID)> saveCallback);

    /**
     * Opens the Library's "add list item" menu.
     */
    void openLibraryAddMenu();

    /**
     * Opens the hamburger menu.
     */
    void openHamburgerMenu();

    /**
     * Opens the "add sprite" dialog.
     *
     * @param spriteImageRelPaths The sprite images to add.
     */
    void openAddSpriteDialog(
        const std::vector<std::string>& spriteImageRelPaths);

    void render() override;

private:
    /**
     * Makes the appropriate windows visible based on the new item's type.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /** Used by this screen's UI. */
    DataModel& dataModel;

    /** The confirmationDialog user's callback. Set while opening the dialog. */
    std::function<void(void)> userOnConfirmation;

    //-------------------------------------------------------------------------
    // Windows
    //-------------------------------------------------------------------------
    /** The left-side window for managing sprite sheets, sprites, etc. */
    LibraryWindow libraryWindow;

    // Edit views
    /** The center stage for editing the active item. */
    SpriteSheetEditView spriteSheetEditView;
    BoundingBoxEditView boundingBoxEditView;
    SpriteEditView spriteEditView;
    AnimationElementsWindow animationElementsWindow;
    AnimationEditView animationEditView;
    GraphicSetEditView graphicSetEditView;
    EntityGraphicSetEditView entityGraphicSetEditView;
    IconEditView iconEditView;

    // Properties windows
    /** The right-side panel for viewing and editing the active item's data. */
    SpriteSheetPropertiesWindow spriteSheetPropertiesWindow;
    BoundingBoxPropertiesWindow boundingBoxPropertiesWindow;
    SpritePropertiesWindow spritePropertiesWindow;
    AnimationPropertiesWindow animationPropertiesWindow;
    GraphicSetPropertiesWindow graphicSetPropertiesWindow;
    EntityGraphicSetPropertiesWindow entityGraphicSetPropertiesWindow;
    IconPropertiesWindow iconPropertiesWindow;

    /** The menu for adding new items that opens when you press the "+". */
    LibraryAddMenu libraryAddMenu;

    /** The hamburger button at the top of the screen, next to the properties
        window. */
    HamburgerButtonWindow hamburgerButtonWindow;

    /** The menu that opens when you press the hamburger button. */
    HamburgerMenu hamburgerMenu;

    /** Confirmation dialog. Child widgets can call openConfirmationDialog()
        to use it. */
    AUI::ConfirmationDialog confirmationDialog;

    /** Dialog for adding a sprite to the active sprite sheet. */
    AddSpriteDialog addSpriteDialog;

    /** Dialog for adding an icon sheet to the IconSheetPanel. */
    AddIconSheetDialog addIconSheetDialog;

    /** Dialog for adding a BoundingBox to the Library. */
    SaveBoundingBoxDialog saveBoundingBoxDialog;
};

} // End namespace ResourceImporter
} // End namespace AM
