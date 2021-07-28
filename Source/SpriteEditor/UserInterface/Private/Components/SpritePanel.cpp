#include "SpritePanel.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{

SpritePanel::SpritePanel(MainScreen& inScreen, SpriteDataModel& inSpriteDataModel)
: AUI::Component(inScreen, "SpritePanel", {-8, 732, 1936, 352})
, mainScreen{inScreen}
, spriteDataModel{inSpriteDataModel}
, backgroundImage(inScreen, "", {0, 0, 1936, 352})
, spriteContainer(inScreen, "SpriteContainer", {191, 24, 1737, 324})
{
    /* Background image */
    backgroundImage.addResolution({1600, 900}, (Paths::TEXTURE_DIR + "SpritePanel/Background_1600.png"));

    /* Container */
    spriteContainer.setNumColumns(10);
    spriteContainer.setCellWidth(156);
    spriteContainer.setCellHeight(162);
}

void SpritePanel::addSprite(const SpriteSheet& sheet, SpriteStaticData& sprite)
{
    std::unique_ptr<AUI::Component> thumbnailPtr{
        std::make_unique<MainThumbnail>(screen, "")};
    MainThumbnail& thumbnail{static_cast<MainThumbnail&>(*thumbnailPtr)};

    thumbnail.thumbnailImage.addResolution({1280, 720}, (spriteDataModel.getWorkingResourcesDir() + sheet.relPath)
        , sprite.textureExtent);
    thumbnail.setText(sprite.displayName);
    thumbnail.setIsSelectable(false);

    // Add a callback to deactivate all other thumbnails when one is activated.
    thumbnail.setOnActivated([&](AUI::Thumbnail* selectedThumb){
        // Deactivate all other thumbnails.
        for (auto& componentPtr : spriteContainer) {
            MainThumbnail& otherThumb = static_cast<MainThumbnail&>(*componentPtr);
            if (otherThumb.getIsActive() && (&otherThumb != selectedThumb)) {
                otherThumb.deactivate();
            }
        }

        // Load the data that this Thumbnail represents as the active sprite.
        mainScreen.loadActiveSprite(&sprite);
    });

    spriteContainer.push_back(std::move(thumbnailPtr));
}

void SpritePanel::refreshActiveSprite(const std::string& newDisplayName)
{
    // Look for an active sprite.
    for (unsigned int i = 0; i < spriteContainer.size(); ++i) {
        AUI::Thumbnail& thumbnail{dynamic_cast<AUI::Thumbnail&>(spriteContainer[i])};
        if (thumbnail.getIsActive()) {
            // Refresh the sprite's display name.
            thumbnail.setText(newDisplayName);
        }
    }
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
    backgroundImage.render(childOffset);

    spriteContainer.render(childOffset);
}

} // End namespace SpriteEditor
} // End namespace AM
