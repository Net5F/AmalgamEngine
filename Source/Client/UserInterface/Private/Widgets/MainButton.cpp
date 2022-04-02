#include "MainButton.h"
#include "AssetCache.h"
#include "Paths.h"

namespace AM
{
namespace Client
{
MainButton::MainButton(AssetCache& assetCache, const SDL_Rect& inScreenExtent,
                       const std::string& inText,
                       const std::string& inDebugName)
: AUI::Button(inScreenExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.addResolution(
        {1600, 900},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "MainButton/Normal.png"));
    hoveredImage.addResolution(
        {1600, 900},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "MainButton/Hovered.png"));
    pressedImage.addResolution(
        {1600, 900},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "MainButton/Pressed.png"));

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace Client
} // End namespace AM
