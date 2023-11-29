#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "MainButton.h"
#include "MainTextInput.h"

namespace AM
{
namespace SpriteEditor
{
class DataModel;

/**
 * A confirmation dialog with header text, body text, and confirm/cancel
 * buttons.
 */
class AddSheetDialog : public AUI::Window
{
public:
    AddSheetDialog(DataModel& ineDataModel);

    virtual ~AddSheetDialog() = default;

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

    // Path entry
    // Note: The path must be relative to SpriteDataModel::workingTexturesDir.
    AUI::Text pathLabel;
    MainTextInput pathInput;

    // Sprite width entry
    AUI::Text widthLabel;
    MainTextInput widthInput;

    // Sprite height entry
    AUI::Text heightLabel;
    MainTextInput heightInput;

    // Sprite Y offset entry
    AUI::Text offsetLabel;
    MainTextInput offsetInput;

    // Base name entry
    AUI::Text nameLabel;
    MainTextInput nameInput;

    /** Right-side confirmation button. Adds the sheet. */
    MainButton addButton;

    /** Left-side cancel button. Closes the dialog without performing any
        action. */
    MainButton cancelButton;

private:
    /**
     * Clears the text in all of this dialog's text inputs and error text.
     */
    void clear();

    /** Used to update the model when a sheet is added. */
    DataModel& dataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    /** Error text, appears if the user tries to submit an invalid input. */
    AUI::Text errorText;
};

} // End namespace SpriteEditor
} // End namespace AM
