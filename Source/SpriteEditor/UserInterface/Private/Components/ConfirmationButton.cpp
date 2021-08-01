#include "ConfirmationButton.h"
#include "AssetCache.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{

ConfirmationButton::ConfirmationButton(AssetCache& assetCache, AUI::Screen& inScreen
                         , const SDL_Rect& inScreenExtent, const std::string& inText, const std::string& inDebugName)
: AUI::Button(inScreen, inScreenExtent, inDebugName)
{
    // Add our backgrounds.
    normalImage.addResolution({1600, 900}, assetCache.loadTexture(
        Paths::TEXTURE_DIR + "ConfirmationButton/Normal.png"));
    hoveredImage.addResolution({1600, 900}, assetCache.loadTexture(
        Paths::TEXTURE_DIR + "ConfirmationButton/Hovered.png"));
    pressedImage.addResolution({1600, 900}, assetCache.loadTexture(
        Paths::TEXTURE_DIR + "ConfirmationButton/Pressed.png"));

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
    text.setText(inText);
}

} // End namespace SpriteEditor
} // End namespace AM
