#include "MainThumbnail.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
MainThumbnail::MainThumbnail(const std::string& inDebugName)
: AUI::Thumbnail({0, 0, 150, 150}, inDebugName)
{
    // Add our backgrounds.
    hoveredImage.setSimpleImage(Paths::TEXTURE_DIR + "Thumbnail/Hovered.png");
    activeImage.setSimpleImage(Paths::TEXTURE_DIR + "Thumbnail/Active.png");
    backdropImage.setSimpleImage(Paths::TEXTURE_DIR + "Thumbnail/Backdrop.png");
    selectedImage.setSimpleImage(Paths::TEXTURE_DIR + "Thumbnail/Selected.png");

    // Move our thumbnail image to the right position.
    thumbnailImage.setLogicalExtent({27, 15, 96, 96});

    // Set our text properties.
    setTextLogicalExtent({13, 120, 123, 20});
    setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 15);
    setTextColor({255, 255, 255, 255});
}

} // End namespace ResourceImporter
} // End namespace AM
