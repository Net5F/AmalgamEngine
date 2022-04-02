#include "TitleButton.h"
#include "AssetCache.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{
TitleButton::TitleButton(AssetCache& assetCache, const SDL_Rect& inScreenExtent,
                         const std::string& inText,
                         const std::string& inDebugName)
: AUI::Button(inScreenExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "Button/Normal.png"));
    hoveredImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "Button/Hovered.png"));
    pressedImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "Button/Pressed.png"));
    disabledImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "Button/Disabled.png"));

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace SpriteEditor
} // End namespace AM
