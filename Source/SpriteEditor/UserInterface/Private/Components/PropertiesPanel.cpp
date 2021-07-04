#include "PropertiesPanel.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{

PropertiesPanel::PropertiesPanel(MainScreen& inScreen)
: AUI::Component(inScreen, "PropertiesPanel", {1617, 0, 315, 461})
, mainScreen{inScreen}
, backgroundImage(inScreen, "", logicalExtent)
{
    /* Background image */
    backgroundImage.addResolution({1920, 1080}, "Textures/PropertiesPanel/Background.png"
                                  , {0, 4, 315, 465});
}

void PropertiesPanel::render(const SDL_Point& parentOffset)
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
    backgroundImage.render(parentOffset);
}

} // End namespace SpriteEditor
} // End namespace AM
