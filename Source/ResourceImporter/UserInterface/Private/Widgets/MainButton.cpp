#include "MainButton.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
MainButton::MainButton(const SDL_Rect& inLogicalExtent,
                       const std::string& inText,
                       const std::string& inDebugName)
: AUI::Button(inLogicalExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.setNineSliceImage(Paths::TEXTURE_DIR + "MainButton/Normal.png",
                                  {2, 2, 2, 2});
    hoveredImage.setNineSliceImage(
        Paths::TEXTURE_DIR + "MainButton/Hovered.png", {2, 2, 2, 2});
    pressedImage.setNineSliceImage(
        Paths::TEXTURE_DIR + "MainButton/Pressed.png", {2, 2, 2, 2});

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace ResourceImporter
} // End namespace AM
