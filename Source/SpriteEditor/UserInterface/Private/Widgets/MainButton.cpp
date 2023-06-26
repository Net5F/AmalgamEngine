#include "MainButton.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{
MainButton::MainButton(const SDL_Rect& inLogicalExtent,
                                       const std::string& inText,
                                       const std::string& inDebugName)
: AUI::Button(inLogicalExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.setSimpleImage(Paths::TEXTURE_DIR
                               + "MainButton/Normal.png");
    hoveredImage.setSimpleImage(Paths::TEXTURE_DIR
                                + "MainButton/Hovered.png");
    pressedImage.setSimpleImage(Paths::TEXTURE_DIR
                                + "MainButton/Pressed.png");

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace SpriteEditor
} // End namespace AM
