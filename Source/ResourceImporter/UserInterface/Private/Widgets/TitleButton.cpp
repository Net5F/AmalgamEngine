#include "TitleButton.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
TitleButton::TitleButton(const SDL_Rect& inLogicalExtent,
                         const std::string& inText,
                         const std::string& inDebugName)
: AUI::Button(inLogicalExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.setNineSliceImage(
        Paths::TEXTURE_DIR + "MainButton/NormalThick.png", {4, 4, 4, 4});
    hoveredImage.setNineSliceImage(
        Paths::TEXTURE_DIR + "MainButton/HoveredThick.png", {4, 4, 4, 4});
    pressedImage.setNineSliceImage(
        Paths::TEXTURE_DIR + "MainButton/PressedThick.png", {4, 4, 4, 4});
    disabledImage.setNineSliceImage(
        Paths::TEXTURE_DIR + "MainButton/DisabledThick.png", {4, 4, 4, 4});

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace ResourceImporter
} // End namespace AM
