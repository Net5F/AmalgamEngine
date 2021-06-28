#pragma once

#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/VerticalGridContainer.h"
#include "ConfirmationButton.h"

namespace AM
{
namespace SpriteEditor
{

/**
 * A confirmation dialog with header text, body text, and confirm/cancel
 * buttons.
 */
class RemSheetDialog : public AUI::Component
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    RemSheetDialog(AUI::Screen& screen, AUI::VerticalGridContainer& inSpritesheetContainer, AUI::Button& inRemSheetButton);

    virtual ~RemSheetDialog() = default;

    /** Background image. */
    AUI::Image backgroundImage;

    /** Body text. Typically will prompt the user with a question that
        describes the decision they're making. */
    AUI::Text bodyText;

    /** Right-side confirmation button. Removes the selected sprite sheet. */
    ConfirmationButton removeButton;

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

    /** Used for disabling remSheetButton when the dialog closes. */
    AUI::Button& remSheetButton;
};

} // End namespace SpriteEditor
} // End namespace AM
