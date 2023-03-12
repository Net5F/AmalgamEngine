#pragma once

#include "AUI/Screen.h"
#include "AUI/Button.h"
#include "AUI/ConfirmationDialog.h"
#include "LibraryWindow.h"
#include "SpriteEditStage.h"
#include "SpritePanel.h"
#include "SaveButtonWindow.h"
#include "PropertiesPanel.h"
#include "TitleButton.h"
#include "AddSheetDialog.h"

namespace AM
{
namespace SpriteEditor
{
class SpriteDataModel;

/**
 * The main screen for doing work.
 */
class MainScreen : public AUI::Screen
{
public:
    MainScreen(SpriteDataModel& inSpriteDataModel);

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
     * Opens the "Add Sheet" dialog.
     */
    void openAddSheetDialog();

    void render() override;

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    /** The sprite data for this project. Used by this screen's UI. */
    SpriteDataModel& spriteDataModel;

    /** The confirmationDialog user's callback. Set while opening the dialog. */
    std::function<void(void)> userOnConfirmation;

    //-------------------------------------------------------------------------
    // Windows
    //-------------------------------------------------------------------------
    /** The left-side window for managing sprite sheets, sprites, etc. */
    LibraryWindow libraryWindow;

    /** The center stage for editing sprite bounding boxes. */
    SpriteEditStage spriteEditStage;

    /** The bottom panel for selecting sprites. */
    SpritePanel spritePanel;

    /** The save button at the top of the screen, next to the properties. */
    SaveButtonWindow saveButtonWindow;

    /** The right-side panel for viewing and editing the active sprite data. */
    PropertiesPanel propertiesPanel;

    /** Confirmation dialog. Child widgets can call openConfirmationDialog()
        to use it. */
    AUI::ConfirmationDialog confirmationDialog;

    /** Dialog for adding a sprite sheet to the SpriteSheetPanel. Child widgets
        can call openAddSheetDialog() to use it. */
    AddSheetDialog addSheetDialog;
};

} // End namespace SpriteEditor
} // End namespace AM
