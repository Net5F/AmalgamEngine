#include "MainThumbnail.h"

namespace AM
{

MainThumbnail::MainThumbnail(AUI::Screen& screen, const char* key)
: AUI::Thumbnail(screen, key, {0, 0, 150, 150})
{
    // Add our backgrounds.
    normalImage.addResolution({1280, 720}, "Textures/Thumbnail/Normal.png");
    hoveredImage.addResolution({1280, 720}, "Textures/Thumbnail/Hovered.png");
    activeImage.addResolution({1280, 720}, "Textures/Thumbnail/Active.png");

    // Move our thumbnail image to the right position.
    thumbnailImage.setLogicalExtent({27, 15, 96, 96});

    // Set our text properties.
    setTextLogicalExtent({13, 120, 123, 20});
    setTextFont("Fonts/B612-Regular.ttf", 15);
    setTextColor({255, 255, 255, 255});
}

} // End namespace AM
