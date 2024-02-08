#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Button.h"

namespace AM
{
namespace ResourceImporter
{

/**
 * A menu that lets the user add items to the Library.
 *
 * Opens when you press the Library's "+" button.
 */
class LibraryAddMenu : public AUI::Window
{
public:
    LibraryAddMenu();

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** The menu's background */
    AUI::Image backgroundImage;

    AUI::Button addBoundingBoxButton;
    AUI::Button addSpriteSheetButton;
    AUI::Button addFloorButton;
    AUI::Button addFloorCoveringButton;
    AUI::Button addWallButton;
    AUI::Button addObjectButton;
    AUI::Button addIconButton;

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void onFocusLost(AUI::FocusLostType focusLostType) override;

private:
    /**
     * Styles the given button and sets its text to the given text.
     */
    void styleButton(AUI::Button& button, const std::string& text);
};

} // End namespace ResourceImporter
} // End namespace AM
