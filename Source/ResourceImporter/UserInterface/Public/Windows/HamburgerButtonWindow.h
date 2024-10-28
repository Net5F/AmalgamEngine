#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Button.h"

namespace AM
{
namespace ResourceImporter
{
class MainScreen;

/**
 * Holds the "hamburger menu" button at the top of the screen, next to the 
 * properties window.
 *
 * Facilitates file operations like saving the json, or exporting sprite 
 * sheet images.
 */
class HamburgerButtonWindow : public AUI::Window
{
public:
    HamburgerButtonWindow(MainScreen& inScreen);

private:
    /** Used to open the confirmation dialog. */
    MainScreen& mainScreen;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Button hamburgerButton;
};

} // End namespace ResourceImporter
} // End namespace AM
