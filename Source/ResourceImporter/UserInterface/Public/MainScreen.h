#pragma once

#include "AUI/Screen.h"
#include "AUI/Button.h"
#include "AUI/ConfirmationDialog.h"
#include "LibraryItemData.h"
#include "LibraryWindow.h"
#include "SaveButtonWindow.h"
#include "BoundingBoxEditStage.h"
#include "BoundingBoxPropertiesWindow.h"
#include "SpriteEditStage.h"
#include "SpritePropertiesWindow.h"
#include "AnimationEditStage.h"
#include "AnimationPropertiesWindow.h"
#include "GraphicSetEditStage.h"
#include "GraphicSetPropertiesWindow.h"
#include "IconEditStage.h"
#include "IconPropertiesWindow.h"
#include "TitleButton.h"
#include "LibraryAddMenu.h"
#include "AddSpriteSheetDialog.h"
#include "AddIconSheetDialog.h"

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
     * Opens a confirmation dialog.
     *
     * @param bodyText  The main dialog text.
     * @param confirmButtonText  The text on the confirm button.
     * @param onConfirmation  Called when the user presses the confirm button.
     */
    void openConfirmationDialog(const std::string& bodyText,
                                const std::string& confirmButtonText,
                                std::function<void(void)> onConfirmation);

    /**
     * Opens the Library's "add list item" menu.
     */
    void openLibraryAddMenu();

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

    /** The menu for adding new items that opens when you press the "+". */
    LibraryAddMenu libraryAddMenu;

    /** The save button at the top of the screen, next to the properties. */
    SaveButtonWindow saveButtonWindow;

    // Edit stages
    /** The center stage for editing the active item. */
    BoundingBoxEditStage boundingBoxEditStage;
    SpriteEditStage spriteEditStage;
    AnimationEditStage animationEditStage;
    GraphicSetEditStage graphicSetEditStage;
    IconEditStage iconEditStage;

    // Properties windows
    /** The right-side panel for viewing and editing the active item's data. */
    BoundingBoxPropertiesWindow boundingBoxPropertiesWindow;
    SpritePropertiesWindow spritePropertiesWindow;
    AnimationPropertiesWindow animationPropertiesWindow;
    GraphicSetPropertiesWindow graphicSetPropertiesWindow;
    IconPropertiesWindow iconPropertiesWindow;

    /** Confirmation dialog. Child widgets can call openConfirmationDialog()
        to use it. */
    AUI::ConfirmationDialog confirmationDialog;

    /** Dialog for adding a sprite sheet to the SpriteSheetPanel. */
    AddSpriteSheetDialog addSpriteSheetDialog;

    /** Dialog for adding an icon sheet to the IconSheetPanel. */
    AddIconSheetDialog addIconSheetDialog;
};

} // End namespace ResourceImporter
} // End namespace AM
