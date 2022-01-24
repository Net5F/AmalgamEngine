#pragma once

#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/VerticalGridContainer.h"
#include "ConfirmationButton.h"
#include "MainTextInput.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
class MainScreen;
class SpriteDataModel;

/**
 * A confirmation dialog with header text, body text, and confirm/cancel
 * buttons.
 */
class AddSheetDialog : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    AddSheetDialog(AssetCache& assetCache, MainScreen& inScreen,
                   AUI::VerticalGridContainer& inSpriteSheetContainer,
                   SpriteDataModel& inSpriteDataModel);

    virtual ~AddSheetDialog() = default;

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

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& parentOffset = {}) override;

private:
    /**
     * Clears the text in all of this dialog's text inputs and error text.
     */
    void clear();

    /** Used to update the UI after sprite data changes. Widget maintains an
        AUI::Screen reference that we could cast, but we want to explicitly
        model a dependency on MainScreen. */
    MainScreen& mainScreen;

    /** Used to remove the currently selected thumbnail when removeButton is
        pressed. */
    AUI::VerticalGridContainer& spriteSheetContainer;

    /** Used to update the model when a sheet is added. */
    SpriteDataModel& spriteDataModel;

    /** Error text, appears if the user tries to submit an invalid input. */
    AUI::Text errorText;
};

} // End namespace SpriteEditor
} // End namespace AM
