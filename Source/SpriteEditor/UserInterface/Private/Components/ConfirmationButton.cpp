#include "ConfirmationButton.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{

ConfirmationButton::ConfirmationButton(AUI::Screen& screen, const char* key
                         , const SDL_Rect& screenExtent, const std::string& inText)
: AUI::Button(screen, key, screenExtent)
{
    // Add our backgrounds.
    normalImage.addResolution({1600, 900}, (Paths::TEXTURE_DIR + "ConfirmationButton/Normal.png"));
    hoveredImage.addResolution({1600, 900}, (Paths::TEXTURE_DIR + "ConfirmationButton/Hovered.png"));
    pressedImage.addResolution({1600, 900}, (Paths::TEXTURE_DIR + "ConfirmationButton/Pressed.png"));

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace SpriteEditor
} // End namespace AM
