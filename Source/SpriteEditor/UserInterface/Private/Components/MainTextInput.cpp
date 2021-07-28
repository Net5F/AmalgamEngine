#include "MainTextInput.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{

MainTextInput::MainTextInput(AUI::Screen& screen, const char* key
                         , const SDL_Rect& screenExtent)
: AUI::TextInput(screen, key, screenExtent)
{
    // Add our backgrounds.
    normalImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "TextInput/Normal.png"));
    hoveredImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "TextInput/Hovered.png"));
    selectedImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "TextInput/Selected.png"));
    disabledImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "TextInput/Disabled.png"));

    // Set our text properties.
    setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 25);

    // Set our input box properties.
    setCursorWidth(2);
}

} // End namespace SpriteEditor
} // End namespace AM
