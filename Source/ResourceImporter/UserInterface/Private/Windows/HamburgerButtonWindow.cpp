#include "HamburgerButtonWindow.h"
#include "MainScreen.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
HamburgerButtonWindow::HamburgerButtonWindow(MainScreen& inScreen)
: AUI::Window({1537, 0, 116, 58}, "HamburgerButtonWindow")
, mainScreen{inScreen}
, backgroundImage{{0, 0, 58, 58}}
, hamburgerButton({0, 0, 58, 58}, "HamburgerButton")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(hamburgerButton);

    /* Window setup */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});

    /* Buttons. */
    hamburgerButton.normalImage.setSimpleImage(
        Paths::TEXTURE_DIR + "HamburgerButton/Normal.png", SDL_ScaleModeLinear);
    hamburgerButton.hoveredImage.setSimpleImage(
        Paths::TEXTURE_DIR + "HamburgerButton/Hovered.png",
        SDL_ScaleModeLinear);
    hamburgerButton.pressedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "HamburgerButton/Pressed.png",
        SDL_ScaleModeLinear);
    hamburgerButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    hamburgerButton.text.setText("");

    hamburgerButton.setOnPressed([this]() { mainScreen.openHamburgerMenu(); });
}

} // End namespace ResourceImporter
} // End namespace AM
