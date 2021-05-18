#include "TitleButton.h"

namespace AM
{

TitleButton::TitleButton(AUI::Screen& screen, const char* key
                         , const SDL_Rect& screenExtent, const std::string& inText)
: AUI::Button(screen, key, screenExtent)
{
    // Add our backgrounds.
    normalImage.addResolution({1280, 720}, "Textures/Button/Normal.png");
    hoveredImage.addResolution({1280, 720}, "Textures/Button/Hovered.png");
    pressedImage.addResolution({1280, 720}, "Textures/Button/Pressed.png");
    disabledImage.addResolution({1280, 720}, "Textures/Button/Disabled.png");

    // Set our text properties.
    text.setFont("Fonts/B612-Regular.ttf", 25);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace AM
