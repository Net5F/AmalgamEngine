#pragma once

#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/VerticalGridContainer.h"
#include "ConfirmationButton.h"
#include "MainTextInput.h"

namespace AM {

/**
 * A confirmation dialog with header text, body text, and confirm/cancel
 * buttons.
 */
class AddSheetDialog : public AUI::Component
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    AddSheetDialog(AUI::Screen& screen, AUI::VerticalGridContainer& inSpritesheetContainer);

    virtual ~AddSheetDialog() = default;

    /** Background image. */
    AUI::Image backgroundImage;

    /** Header text. Typically will prompt the user with text that describes
        the decision they're making. */
    AUI::Text headerText;

    // Path entry
    AUI::Text pathLabel;
    MainTextInput pathInput;
    ConfirmationButton browseButton;

    // Sprite width entry
    AUI::Text widthLabel;
    MainTextInput widthInput;

    // Sprite height entry
    AUI::Text heightLabel;
    MainTextInput heightInput;

    // Base name entry
    AUI::Text nameLabel;
    MainTextInput nameInput;

    /** Right-side confirmation button. Performs the action in question. */
    ConfirmationButton addButton;

    /** Left-side cancel button. Closes the dialog without performing any
        action. */
    ConfirmationButton cancelButton;

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& parentOffset = {}) override;

private:
    /** Used to remove the currently selected thumbnail when removeButton is
        pressed. */
    AUI::VerticalGridContainer& spritesheetContainer;
};

} // namespace AM
