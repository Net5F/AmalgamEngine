#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "ConfirmationButton.h"
#include "MainTextInput.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
class SpriteDataModel;

/**
 * A confirmation dialog with header text, body text, and confirm/cancel
 * buttons.
 */
class AddSheetDialog : public AUI::Window
{
public:
    AddSheetDialog(AssetCache& assetCache,
                   SpriteDataModel& inSpriteDataModel);

    virtual ~AddSheetDialog() = default;

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** The dialog's background */
    AUI::Image backgroundImage;

    /** The header text at the top of the dialog. */
    AUI::Text headerText;

    // Path entry
    // Note: The path must be relative to Core::resourcePath.
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
    ConfirmationButton addButton;

    /** Left-side cancel button. Closes the dialog without performing any
        action. */
    ConfirmationButton cancelButton;

private:
    /**
     * Clears the text in all of this dialog's text inputs and error text.
     */
    void clear();

    /** Used to update the model when a sheet is added. */
    SpriteDataModel& spriteDataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    /** Error text, appears if the user tries to submit an invalid input. */
    AUI::Text errorText;
};

} // End namespace SpriteEditor
} // End namespace AM
