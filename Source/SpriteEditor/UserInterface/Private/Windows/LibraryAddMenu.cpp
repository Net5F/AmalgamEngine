#include "LibraryAddMenu.h"
#include "MainScreen.h"
#include "Paths.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{
LibraryAddMenu::LibraryAddMenu()
: AUI::Window({310, 5, 169, 187}, "LibraryAddMenu")
, backgroundImage({0, 0, logicalExtent.w, logicalExtent.h})
, addSpriteSheetButton({1, 1, 167, 32}, "AddSpriteSheetButton")
, addFloorButton({1, 32, 167, 32}, "AddFloorButton")
, addFloorCoveringButton({1, 64, 167, 32}, "AddFloorCoveringButton")
, addWallButton({1, 96, 167, 32}, "AddWallButton")
, addObjectButton({1, 128, 167, 32}, "AddObjectButton")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(addSpriteSheetButton);
    children.push_back(addFloorButton);
    children.push_back(addFloorCoveringButton);
    children.push_back(addWallButton);
    children.push_back(addObjectButton);

    // Flag ourselves as focusable, so we can close when focus is lost.
    isFocusable = true;

    /* Background image. */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});

    /* Add sprite sheet button. */
    styleButton(addSpriteSheetButton, "SpriteSheet");
    styleButton(addFloorButton, "Floor");
    styleButton(addFloorCoveringButton, "Floor Covering");
    styleButton(addWallButton, "Wall");
    styleButton(addObjectButton, "Object");
}

void LibraryAddMenu::onFocusLost(AUI::FocusLostType focusLostType)
{
    // When we lose focus, close the menu.
    setIsVisible(false);
}

void LibraryAddMenu::styleButton(AUI::Button& button, const std::string& text)
{
    button.text.setLogicalExtent({10, 0, (167 - 10), 31});
    button.hoveredImage.setSimpleImage(Paths::TEXTURE_DIR
                                       + "Highlights/Hovered.png");
    button.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    button.text.setColor({255, 255, 255, 255});
    button.text.setText(text);
    button.text.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Left);
}

} // End namespace SpriteEditor
} // End namespace AM
