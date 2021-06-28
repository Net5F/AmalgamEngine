#include "ConfirmationButton.h"

namespace AM
{
namespace SpriteEditor
{

ConfirmationButton::ConfirmationButton(AUI::Screen& screen, const char* key
                         , const SDL_Rect& screenExtent, const std::string& inText)
: AUI::Button(screen, key, screenExtent)
{
    // Add our backgrounds.
    normalImage.addResolution({1600, 900}, "Textures/ConfirmationButton/Normal.png");
    hoveredImage.addResolution({1600, 900}, "Textures/ConfirmationButton/Hovered.png");
    pressedImage.addResolution({1600, 900}, "Textures/ConfirmationButton/Pressed.png");

    // Set our text properties.
    text.setFont("Fonts/B612-Regular.ttf", 18);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace SpriteEditor
} // End namespace AM
