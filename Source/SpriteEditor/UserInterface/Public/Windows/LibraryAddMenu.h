#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Button.h"

namespace AM
{
namespace SpriteEditor
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

    AUI::Button addSpriteSheetButton;

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                            const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition) override;

    void onFocusLost(AUI::FocusLostType focusLostType) override;
};

} // End namespace SpriteEditor
} // End namespace AM
