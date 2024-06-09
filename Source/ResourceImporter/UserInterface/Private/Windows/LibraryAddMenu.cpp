#include "LibraryAddMenu.h"
#include "MainScreen.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
LibraryAddMenu::LibraryAddMenu()
: AUI::Window({310, 5, 169, 291}, "LibraryAddMenu")
, backgroundImage({0, 0, logicalExtent.w, logicalExtent.h})
, addBoundingBoxButton({1, 1, 167, 32}, "AddBoundingBoxButton")
, addSpriteSheetButton({1, 33, 167, 32}, "AddSpriteSheetButton")
, addAnimationButton({1, 65, 167, 32}, "AddAnimationButton")
, addTerrainButton({1, 129, 167, 32}, "AddTerrainButton")
, addFloorButton({1, 97, 167, 32}, "AddFloorButton")
, addWallButton({1, 161, 167, 32}, "AddWallButton")
, addObjectButton({1, 193, 167, 32}, "AddObjectButton")
, addEntityButton({1, 225, 167, 32}, "AddEntityButton")
, addIconButton({1, 257, 167, 32}, "AddIconButton")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(addBoundingBoxButton);
    children.push_back(addSpriteSheetButton);
    children.push_back(addAnimationButton);
    children.push_back(addTerrainButton);
    children.push_back(addFloorButton);
    children.push_back(addWallButton);
    children.push_back(addObjectButton);
    children.push_back(addEntityButton);
    children.push_back(addIconButton);

    // Flag ourselves as focusable, so we can close when focus is lost.
    isFocusable = true;

    /* Background image. */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});

    /* Buttons. */
    styleButton(addBoundingBoxButton, "Bounding Box");
    styleButton(addSpriteSheetButton, "Sprite Sheet");
    styleButton(addAnimationButton, "Animation");
    styleButton(addTerrainButton, "Terrain");
    styleButton(addFloorButton, "Floor");
    styleButton(addWallButton, "Wall");
    styleButton(addObjectButton, "Object");
    styleButton(addEntityButton, "Entity");
    styleButton(addIconButton, "Icon");
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

} // End namespace ResourceImporter
} // End namespace AM
