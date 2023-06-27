#include "SpriteSetSlot.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{
SpriteSetSlot::SpriteSetSlot()
: AUI::Widget({0, 0, 180, 255}, "SpriteSetSlot")
, topText{{0, 0, logicalExtent.w, 27}}
, checkerboardImage{{30, 45, 120, 120}}
, spriteImage{{30, 45, 120, 120}}
, spriteNameText{{0, 175, logicalExtent.w, 27}}
, assignButton{{34, 208, 112, 38}, "ASSIGN"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(checkerboardImage);
    children.push_back(spriteImage);
    children.push_back(spriteNameText);
    children.push_back(assignButton);

    // Set our text properties.
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 24);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);

    spriteNameText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    spriteNameText.setColor({255, 255, 255, 255});
    spriteNameText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);

    // Set our background image.
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditStage/Checkerboard.png");
}

} // End namespace SpriteEditor
} // End namespace AM
