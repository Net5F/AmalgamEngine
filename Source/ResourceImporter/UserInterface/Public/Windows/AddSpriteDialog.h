#pragma once

#include "LibraryItemData.h"
#include "MainButton.h"
#include "MainTextInput.h"
#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/Checkbox.h"

namespace AM
{
namespace ResourceImporter
{
class MainScreen;
class DataModel;

/**
 * A confirmation dialog for adding sprite sheets.
 */
class AddSpriteDialog : public AUI::Window
{
public:
    AddSpriteDialog(MainScreen& inScreen, DataModel& inDataModel);

    virtual ~AddSpriteDialog() = default;

    /**
     * Sets the sprite image paths to add to the active sprite sheet the next 
     * time the "Add" button is pressed.
     */
    void setSpriteImageRelPaths(
        const std::vector<std::string>& inSpriteImageRelPaths);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** Semi-transparent shadow image to obscure things that are behind the
        dialog. */
    AUI::Image shadowImage;

    /** The dialog's background image. */
    AUI::Image backgroundImage;

    /** The header text at the top of the dialog. */
    AUI::Text headerText;

    // Stage origin entry
    AUI::Text stageOriginXLabel;
    MainTextInput stageOriginXInput;
    AUI::Text stageOriginYLabel;
    MainTextInput stageOriginYInput;

    // Premultiply alpha entry
    AUI::Text premultiplyAlphaLabel;
    AUI::Checkbox premultiplyAlphaInput;

    /** Right-side confirmation button. Adds the sprites. */
    MainButton addButton;

    /** Left-side cancel button. Closes the dialog without performing any
        action. */
    MainButton cancelButton;

private:
    /**
     * If the new active item is a sprite sheet, saves its ID.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * Clears the text in all of this dialog's text inputs and error text.
     */
    void clear();

    /** Used to open the error dialog when adding a sprite fails. */
    MainScreen& mainScreen;

    /** Used to update the model when a sheet is added. */
    DataModel& dataModel;

    /** The active sprite sheet's ID. */
    SpriteSheetID activeSpriteSheetID;

    /** The sprite images to add to the active sprite sheet the next time the 
        Add button is pressed. */
    std::vector<std::string> spriteImageRelPaths;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    /** Error text, appears if the user tries to submit an invalid input. */
    AUI::Text errorText;
};

} // namespace ResourceImporter
} // End namespace AM
