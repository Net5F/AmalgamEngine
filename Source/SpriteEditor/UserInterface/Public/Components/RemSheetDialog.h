#pragma once

#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/VerticalGridContainer.h"
#include "ConfirmationButton.h"

namespace AM
{
namespace SpriteEditor
{

class MainScreen;
class SpriteDataModel;

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
    RemSheetDialog(MainScreen& inScreen, AUI::VerticalGridContainer& inSpriteSheetContainer
                   , AUI::Button& inRemSheetButton, SpriteDataModel& inSpriteDataModel);

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
    /** Used to update the UI after sprite data changes. Component maintains an
        AUI::Screen reference that we could cast, but we want to explicitly
        model a dependency on MainScreen. */
    MainScreen& mainScreen;

    /** Used to remove the currently selected thumbnail when removeButton is
        pressed. */
    AUI::VerticalGridContainer& spriteSheetContainer;

    /** Used for disabling remSheetButton when the dialog closes. */
    AUI::Button& remSheetButton;

    /** Used to update the model when a sheet is removed. */
    SpriteDataModel& spriteDataModel;
};

} // End namespace SpriteEditor
} // End namespace AM
