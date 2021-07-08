#include "SpriteEditStage.h"
#include "MainScreen.h"

namespace AM
{
namespace SpriteEditor
{

SpriteEditStage::SpriteEditStage(MainScreen& inScreen)
: AUI::Component(inScreen, "SpriteEditStage", {0, 0, 1920, 1080})
, mainScreen{inScreen}
, checkerboardImage(inScreen, "", {846, 210, 213, 409})
{
    checkerboardImage.addResolution({1920, 1080}, "Textures/SpriteEditStage/Checkerboard.png");
}

void SpriteEditStage::render(const SDL_Point& parentOffset)
{
    // Keep our scaling up to date.
    refreshScaling();

    // Save the extent that we're going to render at.
    lastRenderedExtent = scaledExtent;
    lastRenderedExtent.x += parentOffset.x;
    lastRenderedExtent.y += parentOffset.y;

    // Children should render at the parent's offset + this component's offset.
    SDL_Point childOffset{parentOffset};
    childOffset.x += scaledExtent.x;
    childOffset.y += scaledExtent.y;

    // Render our children.
    checkerboardImage.render(childOffset);
}

} // End namespace SpriteEditor
} // End namespace AM
