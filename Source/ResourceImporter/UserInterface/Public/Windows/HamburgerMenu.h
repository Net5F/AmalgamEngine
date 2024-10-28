#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Button.h"

namespace AM
{
namespace ResourceImporter
{

/**
 * A menu that lets the user perform file operations, like saving the json, or 
 * exporting sprite sheet images
 *
 * Opens when you press the hamburger button.
 */
class HamburgerMenu : public AUI::Window
{
public:
    HamburgerMenu();

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** The menu's background */
    AUI::Image backgroundImage;

    AUI::Button saveButton;
    AUI::Button exportButton;

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
