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
, addSpriteSheetButton({1, 1, 167, 31})
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(addSpriteSheetButton);

    // Flag ourselves as focusable, so we can close when focus is lost.
    isFocusable = true;

    /* Background image. */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});

    /* Add sprite sheet button. */
    addSpriteSheetButton.text.setLogicalExtent({10, 0, (167 - 10), 31});
    addSpriteSheetButton.hoveredImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Highlights/Hovered.png");
    addSpriteSheetButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    addSpriteSheetButton.text.setColor({255, 255, 255, 255});
    addSpriteSheetButton.text.setText("Sprite Sheet");
    addSpriteSheetButton.text.setHorizontalAlignment(
        AUI::Text::HorizontalAlignment::Left);
}

AUI::EventResult LibraryAddMenu::onMouseDown(AUI::MouseButtonType buttonType,
                        const SDL_Point& cursorPosition)
{
    // We need to handle mouse downs, so our focus doesn't get dropped.
    return AUI::EventResult{.wasHandled{true}};
}

AUI::EventResult LibraryAddMenu::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                             const SDL_Point& cursorPosition)
{
    return onMouseDown(buttonType, cursorPosition);
}

void LibraryAddMenu::onFocusLost(AUI::FocusLostType focusLostType)
{
    // When we lose focus, close the menu.
    setIsVisible(false);
}

} // End namespace SpriteEditor
} // End namespace AM
