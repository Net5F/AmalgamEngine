#include "TitleButton.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{
TitleButton::TitleButton(const SDL_Rect& inScreenExtent,
                         const std::string& inText,
                         const std::string& inDebugName)
: AUI::Button(inScreenExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.setSimpleImage(Paths::TEXTURE_DIR + "Button/Normal.png");
    hoveredImage.setSimpleImage(Paths::TEXTURE_DIR + "Button/Hovered.png");
    pressedImage.setSimpleImage(Paths::TEXTURE_DIR + "Button/Pressed.png");
    disabledImage.setSimpleImage(Paths::TEXTURE_DIR + "Button/Disabled.png");

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace SpriteEditor
} // End namespace AM
