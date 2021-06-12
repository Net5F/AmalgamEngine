#include "MainThumbnail.h"

namespace AM
{

MainThumbnail::MainThumbnail(AUI::Screen& screen, const char* key
                         , const SDL_Rect& screenExtent)
: AUI::Thumbnail(screen, key, screenExtent)
{
    // Add our backgrounds.
    normalImage.addResolution({1280, 720}, "Textures/Thumbnail/Normal.png");
    hoveredImage.addResolution({1280, 720}, "Textures/Thumbnail/Hovered.png");
    activeImage.addResolution({1280, 720}, "Textures/Thumbnail/Active.png");

    // Move our thumbnail image to the right position.
    thumbnailImage.setLogicalExtent({18, 10, 64, 64});

    // Set our text properties.
    setTextLogicalExtent({9, 80, 82, 20});
    setTextFont("Fonts/B612-Regular.ttf", 10);
    setTextColor({255, 255, 255, 255});
}

} // End namespace AM
