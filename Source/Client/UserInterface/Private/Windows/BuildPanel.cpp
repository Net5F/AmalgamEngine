#include "BuildPanel.h"
#include "MainScreen.h"
#include "AssetCache.h"
#include "SpriteData.h"
#include "BuildOverlay.h"
#include "MainThumbnail.h"
#include "SharedConfig.h"
#include "Paths.h"
#include "Ignore.h"
#include "AMAssert.h"
#include <memory>

namespace AM
{
namespace Client
{
BuildPanel::BuildPanel(AssetCache& inAssetCache,
                       SpriteData& inSpriteData, BuildOverlay& inBuildOverlay)
: AUI::Window{{0, 734, 1920, 346}, "BuildPanel"}
, assetCache{inAssetCache}
, buildOverlay{inBuildOverlay}
, tileLayerIndex{0}
, backgroundImage{{0, 0, 1920, 346}, "BuildPanelBackground"}
, tileContainer{{183, 22, 1554, 324}, "TileContainer"}
, layerLabel{{1792, 22, 90, 28}, "LayerLabel"}
, layerDownButton{inAssetCache, {1767, 62, 64, 28}, "<", "LayerDownButton"}
, layerUpButton{inAssetCache, {1844, 62, 64, 28}, ">", "LayerUpButton"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(tileContainer);
    children.push_back(layerLabel);
    children.push_back(layerDownButton);
    children.push_back(layerUpButton);

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

    /* Layer label */
    layerLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    layerLabel.setColor({255, 255, 255, 255});
    layerLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    layerLabel.setText("Layer " + std::to_string(tileLayerIndex));

    /* Layer buttons */
    layerDownButton.setOnPressed([&]() {
        if (tileLayerIndex > 0) {
            // Update our label.
            tileLayerIndex--;
            layerLabel.setText("Layer " + std::to_string(tileLayerIndex));

            // Update BuildOverlay.
            buildOverlay.setSelectedLayer(tileLayerIndex);
        }
    });

    layerUpButton.setOnPressed([&]() {
        if (tileLayerIndex < SharedConfig::MAX_TILE_LAYERS) {
            // Update our label.
            tileLayerIndex++;
            layerLabel.setText("Layer " + std::to_string(tileLayerIndex));

            // Update BuildOverlay.
            buildOverlay.setSelectedLayer(tileLayerIndex);
        }
    });
}

void BuildPanel::addTile(const Sprite& sprite)
{
    // Construct the new sprite thumbnail.
    std::unique_ptr<AUI::Widget> thumbnailPtr{
        std::make_unique<MainThumbnail>(assetCache, "BuildPanelThumbnail")};
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
