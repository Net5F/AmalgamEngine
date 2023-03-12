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
SpritePanel::SpritePanel(SpriteDataModel& inSpriteDataModel)
: AUI::Window({-8, 732, 1936, 352}, "SpritePanel")
, spriteDataModel{inSpriteDataModel}
, backgroundImage({0, 0, 1936, 352}, "SpritePanelBackground")
, spriteContainer({191, 24, 1737, 324}, "SpriteContainer")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(spriteContainer);

    /* Background image */
    backgroundImage.addResolution(
        {1600, 900}, (Paths::TEXTURE_DIR + "SpritePanel/Background_1600.png"));

    /* Container */
    spriteContainer.setNumColumns(10);
    spriteContainer.setCellWidth(156);
    spriteContainer.setCellHeight(162);

    // When a sprite is added or removed from the model, update this widget.
    spriteDataModel.spriteAdded.connect<&SpritePanel::onSpriteAdded>(*this);
    spriteDataModel.spriteRemoved.connect<&SpritePanel::onSpriteRemoved>(*this);

    // When a sprite's display name is updated, update the matching thumbnail.
    spriteDataModel.spriteDisplayNameChanged
        .connect<&SpritePanel::onSpriteDisplayNameChanged>(*this);
}

void SpritePanel::onSpriteAdded(unsigned int spriteID, const Sprite& sprite)
{
    // Construct the new sprite thumbnail.
    std::unique_ptr<AUI::Widget> thumbnailPtr{
        std::make_unique<MainThumbnail>("SpritePanelThumbnail")};
    MainThumbnail& thumbnail{static_cast<MainThumbnail&>(*thumbnailPtr)};
    thumbnail.setText(sprite.displayName);
    thumbnail.setIsSelectable(false);

    // Load the sprite's image.
    std::string imagePath{spriteDataModel.getWorkingTexturesDir()};
    imagePath += sprite.parentSpriteSheetPath;
    thumbnail.thumbnailImage.addResolution({1280, 720}, imagePath,
                                           sprite.textureExtent);

    // Add a callback to deactivate all other thumbnails when one is activated.
    thumbnail.setOnActivated([this, spriteID](AUI::Thumbnail* selectedThumb) {
        // Deactivate all other thumbnails.
        for (auto& widgetPtr : spriteContainer) {
            MainThumbnail& otherThumb{static_cast<MainThumbnail&>(*widgetPtr)};
            if (otherThumb.getIsActive() && (&otherThumb != selectedThumb)) {
                otherThumb.deactivate();
            }
        }

        // Load the data that this Thumbnail represents as the active sprite.
        spriteDataModel.setActiveSprite(spriteID);
    });

    spriteContainer.push_back(std::move(thumbnailPtr));
    thumbnailMap.emplace(spriteID, &thumbnail);
}

void SpritePanel::onSpriteRemoved(unsigned int sheetID)
{
    auto spriteIt{thumbnailMap.find(sheetID)};
    if (spriteIt == thumbnailMap.end()) {
        LOG_FATAL("Failed to find sprite during removal.");
    }

    // Remove the thumbnail from the container.
    spriteContainer.erase(spriteIt->second);

    // Remove the thumbnail from the map.
    thumbnailMap.erase(spriteIt);
}

void SpritePanel::onSpriteDisplayNameChanged(unsigned int spriteID,
                                             const std::string& newDisplayName)
{
    auto thumbnailIt{thumbnailMap.find(spriteID)};
    if (thumbnailIt == thumbnailMap.end()) {
        LOG_FATAL("Failed to find a thumbnail for the given sprite.");
    }

    MainThumbnail& thumbnail{*(thumbnailIt->second)};
    if (thumbnail.getIsActive()) {
        // Update the thumbnail to use the sprite's new display name.
        thumbnail.setText(newDisplayName);
    }
}

} // End namespace SpriteEditor
} // End namespace AM
