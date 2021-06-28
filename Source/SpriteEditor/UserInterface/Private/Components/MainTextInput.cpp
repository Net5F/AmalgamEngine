#include "MainTextInput.h"

namespace AM
{
namespace SpriteEditor
{

MainTextInput::MainTextInput(AUI::Screen& screen, const char* key
                         , const SDL_Rect& screenExtent)
: AUI::TextInput(screen, key, screenExtent)
{
    // Add our backgrounds.
    normalImage.addResolution({1920, 1080}, "Textures/TextInput/Normal.png");
    hoveredImage.addResolution({1920, 1080}, "Textures/TextInput/Hovered.png");
    selectedImage.addResolution({1920, 1080}, "Textures/TextInput/Selected.png");
    disabledImage.addResolution({1920, 1080}, "Textures/TextInput/Disabled.png");

    // Set our text properties.
    setTextFont("Fonts/B612-Regular.ttf", 25);

    // Set our input box properties.
    setCursorWidth(2);
}

} // End namespace SpriteEditor
} // End namespace AM
