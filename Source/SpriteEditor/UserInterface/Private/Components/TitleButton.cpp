#include "TitleButton.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{

TitleButton::TitleButton(AUI::Screen& screen, const char* key
                         , const SDL_Rect& screenExtent, const std::string& inText)
: AUI::Button(screen, key, screenExtent)
{
    // Add our backgrounds.
    normalImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "Button/Normal.png"));
    hoveredImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "Button/Hovered.png"));
    pressedImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "Button/Pressed.png"));
    disabledImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "Button/Disabled.png"));

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace SpriteEditor
} // End namespace AM
