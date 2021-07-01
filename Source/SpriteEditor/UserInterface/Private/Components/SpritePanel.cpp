#include "SpritePanel.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "Ignore.h"

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

    for (unsigned int i = 0; i < 22; ++i) {
        addSprite("Textures/Temp/iso_test_sprites.png");
    }

    // Register for the events that we want to listen for.
    registerListener(AUI::InternalEvent::MouseButtonDown);
}

void SpritePanel::addSprite(const std::string& thumbPath)
{
    std::unique_ptr<AUI::Component> thumbnailPtr{
        std::make_unique<MainThumbnail>(screen, "")};
    MainThumbnail& thumbnail{static_cast<MainThumbnail&>(*thumbnailPtr)};

    thumbnail.thumbnailImage.addResolution({1280, 720}, thumbPath);
    thumbnail.setText(thumbPath);
    thumbnail.setIsSelectable(false);

    // Add a callback to deactivate all other components when this one
    // is selected.
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

bool SpritePanel::onMouseButtonDown(SDL_MouseButtonEvent& event)
{
    // If the click event was outside our extent.
    if (!(containsPoint({event.x, event.y}))) {
        // Deselect any selected component.
        for (auto& componentPtr : spriteContainer) {
            MainThumbnail& thumbnail = static_cast<MainThumbnail&>(*componentPtr);
            if (thumbnail.getIsSelected()) {
                thumbnail.deselect();
                break;
            }
        }
    }

    return false;
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
