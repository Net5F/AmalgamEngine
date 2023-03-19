#include "ConfirmationButton.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{
ConfirmationButton::ConfirmationButton(const SDL_Rect& inScreenExtent,
                                       const std::string& inText,
                                       const std::string& inDebugName)
: AUI::Button(inScreenExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.setSimpleImage(Paths::TEXTURE_DIR + "ConfirmationButton/Normal.png");
    hoveredImage.setSimpleImage(Paths::TEXTURE_DIR + "ConfirmationButton/Hovered.png");
    pressedImage.setSimpleImage(Paths::TEXTURE_DIR + "ConfirmationButton/Pressed.png");

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace SpriteEditor
} // End namespace AM
