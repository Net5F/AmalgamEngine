#include "MainThumbnail.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{

MainThumbnail::MainThumbnail(AUI::Screen& inScreen, const std::string& inDebugName)
: AUI::Thumbnail(inScreen, {0, 0, 150, 150}, inDebugName)
{
    // Add our backgrounds.
    hoveredImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "Thumbnail/Hovered.png"));
    activeImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "Thumbnail/Active.png"));
    backdropImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "Thumbnail/Backdrop.png"));
    selectedImage.addResolution({1920, 1080}, (Paths::TEXTURE_DIR + "Thumbnail/Selected.png"));

    // Move our thumbnail image to the right position.
    thumbnailImage.setLogicalExtent({27, 15, 96, 96});

    // Set our text properties.
    setTextLogicalExtent({13, 120, 123, 20});
    setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 15);
    setTextColor({255, 255, 255, 255});
}

} // End namespace SpriteEditor
} // End namespace AM
