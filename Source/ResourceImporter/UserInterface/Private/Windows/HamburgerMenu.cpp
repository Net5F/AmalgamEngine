#include "HamburgerMenu.h"
#include "MainScreen.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
HamburgerMenu::HamburgerMenu()
: AUI::Window({1368, 13, 169, 66}, "HamburgerMenu")
, backgroundImage({0, 0, logicalExtent.w, logicalExtent.h})
, saveButton({1, 1, 167, 32}, "SaveButton")
, exportButton({1, 33, 167, 32}, "ExportButton")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(saveButton);
    children.push_back(exportButton);

    // Flag ourselves as focusable, so we can close when focus is lost.
    isFocusable = true;

    /* Background image. */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});

    /* Buttons. */
    styleButton(saveButton, "Save");
    styleButton(exportButton, "Export");
}

void HamburgerMenu::onFocusLost(AUI::FocusLostType focusLostType)
{
    // When we lose focus, close the menu.
    setIsVisible(false);
}

void HamburgerMenu::styleButton(AUI::Button& button, const std::string& text)
{
    button.text.setLogicalExtent({10, 0, (167 - 10), 31});
    button.hoveredImage.setSimpleImage(Paths::TEXTURE_DIR
                                       + "Highlights/Hovered.png");
    button.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    button.text.setColor({255, 255, 255, 255});
    button.text.setText(text);
    button.text.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Left);
}

} // End namespace ResourceImporter
} // End namespace AM
