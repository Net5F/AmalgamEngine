#include "SpritePanel.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "Log.h"

namespace AM
{
namespace SpriteEditor
{

SpritePanel::SpritePanel(AUI::Screen& inScreen)
: AUI::Component(inScreen, "SpritePanel", {-8, 732, 1936, 352})
, backgroundImage(inScreen, "", logicalExtent)
, spriteContainer(inScreen, "SpriteContainer", {183, 756, 1737, 324})
{
    /* Background image */
    backgroundImage.addResolution({1600, 900}, "Textures/SpritePanel/Background_1600.png");

    /* Container */
    spriteContainer.setNumColumns(10);
    spriteContainer.setCellWidth(156);
    spriteContainer.setCellHeight(162);
}

void SpritePanel::addSprite(const SpriteSheet& sheet, const SpriteStaticData& sprite)
{
    std::unique_ptr<AUI::Component> thumbnailPtr{
        std::make_unique<MainThumbnail>(screen, "")};
    MainThumbnail& thumbnail{static_cast<MainThumbnail&>(*thumbnailPtr)};

    thumbnail.thumbnailImage.addResolution({1280, 720}, sheet.relPath
        , sprite.textureExtent);
    thumbnail.setText(sprite.displayName);
    thumbnail.setIsSelectable(false);

    // Add a callback to deactivate all other components when this one
    // is activated.
    thumbnail.setOnActivated([&](AUI::Thumbnail* selectedThumb){
        // Deactivate all other thumbnails.
        for (auto& componentPtr : spriteContainer) {
            MainThumbnail& otherThumb = static_cast<MainThumbnail&>(*componentPtr);
            if (otherThumb.getIsActive() && (&otherThumb != selectedThumb)) {
                otherThumb.deactivate();
            }
        }
    });

    spriteContainer.push_back(std::move(thumbnailPtr));
}

void SpritePanel::clearSprites()
{
    spriteContainer.clear();
}

void SpritePanel::render(const SDL_Point& parentOffset)
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

    spriteContainer.render(parentOffset);
}

} // End namespace SpriteEditor
} // End namespace AM
