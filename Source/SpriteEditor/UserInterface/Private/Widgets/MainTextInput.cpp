#include "MainTextInput.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{
MainTextInput::MainTextInput(const SDL_Rect& inScreenExtent,
                             const std::string& inDebugName)
: AUI::TextInput(inScreenExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.addResolution({1920, 1080},
                              (Paths::TEXTURE_DIR + "TextInput/Normal.png"));
    hoveredImage.addResolution({1920, 1080},
                               (Paths::TEXTURE_DIR + "TextInput/Hovered.png"));
    focusedImage.addResolution({1920, 1080},
                               (Paths::TEXTURE_DIR + "TextInput/Selected.png"));
    disabledImage.addResolution(
        {1920, 1080}, (Paths::TEXTURE_DIR + "TextInput/Disabled.png"));

    // Set our text properties.
    setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 25);

    // Set our input box properties.
    setCursorWidth(2);
}

} // End namespace SpriteEditor
} // End namespace AM
