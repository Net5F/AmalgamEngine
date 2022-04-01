#include "BuildPanel.h"
#include "MainScreen.h"
#include "AssetCache.h"
#include "SpriteData.h"
#include "BuildOverlay.h"
#include "MainThumbnail.h"
#include "Paths.h"
#include "Ignore.h"
#include "AMAssert.h"
#include <memory>

namespace AM
{
namespace Client
{
BuildPanel::BuildPanel(AssetCache& inAssetCache, MainScreen& inScreen,
                       SpriteData& inSpriteData, BuildOverlay& inBuildOverlay)
: AUI::Window(inScreen, {0, 734, 1920, 346}, "BuildPanel")
, assetCache{inAssetCache}
, buildOverlay{inBuildOverlay}
, backgroundImage(inScreen, {0, 0, 1920, 346}, "BuildPanelBackground")
, tileContainer(inScreen, {183, 22, 1554, 324}, "TileContainer")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(tileContainer);

    /* Background image */
    backgroundImage.addResolution(
        {1600, 900},
        inAssetCache.loadTexture(Paths::TEXTURE_DIR
                               + "SpritePanel/Background_1600.png"),
                               {7, 0, 1600, 290});

    /* Container */
    tileContainer.setNumColumns(10);
    tileContainer.setCellWidth(156);
    tileContainer.setCellHeight(162);

    // TODO: We need some tags on our sprites to tell us which ones can be
    //       used as tiles.
    // Fill the container with the available tiles.
    for (const Sprite& sprite : inSpriteData.getAllSprites()) {
        // Skip the empty sprite.
        // TODO: Once tags are added, we can remove this check since the
        //       empty sprite will be naturally filtered out.
        if (sprite.numericID == -1) {
            continue;
        }

        addTile(sprite);
    }
}

void BuildPanel::addTile(const Sprite& sprite)
{
    // Construct the new sprite thumbnail.
    std::unique_ptr<AUI::Widget> thumbnailPtr{
        std::make_unique<MainThumbnail>(assetCache, screen, "BuildPanelThumbnail")};
    MainThumbnail& thumbnail{static_cast<MainThumbnail&>(*thumbnailPtr)};
    thumbnail.setText(sprite.displayName);
    thumbnail.setIsActivateable(false);

    // Load the sprite's image.
    thumbnail.thumbnailImage.addResolution(
        {1280, 720}, sprite.texture, sprite.textureExtent);

    // Add a callback to deactivate all other thumbnails when one is activated.
    thumbnail.setOnSelected([this, &sprite](AUI::Thumbnail* selectedThumb) {
        // Deactivate all other thumbnails.
        for (auto& widgetPtr : tileContainer) {
            MainThumbnail& otherThumb
                = static_cast<MainThumbnail&>(*widgetPtr);
            if (otherThumb.getIsSelected() && (&otherThumb != selectedThumb)) {
                otherThumb.deselect();
            }
        }

        // Tell BuildOverlay that the active tile changed.
        buildOverlay.setSelectedTile(sprite);
    });

    tileContainer.push_back(std::move(thumbnailPtr));
}

} // End namespace Client
} // End namespace AM
