#include "MainTextInput.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
MainTextInput::MainTextInput(const SDL_Rect& inLogicalExtent,
                             const std::string& inDebugName)
: AUI::TextInput(inLogicalExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.setSimpleImage(Paths::TEXTURE_DIR + "TextInput/Normal.png");
    hoveredImage.setSimpleImage(Paths::TEXTURE_DIR + "TextInput/Hovered.png");
    focusedImage.setSimpleImage(Paths::TEXTURE_DIR + "TextInput/Selected.png");
    disabledImage.setSimpleImage(Paths::TEXTURE_DIR + "TextInput/Disabled.png");

    // Set our text properties.
    setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 25);

    // Set our input box properties.
    setCursorWidth(2);
}

} // End namespace ResourceImporter
} // End namespace AM
