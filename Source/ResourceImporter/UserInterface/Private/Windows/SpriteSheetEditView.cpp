#include "SpriteSheetEditView.h"
#include "MainScreen.h"
#include "EditorSpriteSheet.h"
#include "DataModel.h"
#include "Paths.h"
#include "AUI/ScalingHelpers.h"
#include "AUI/Core.h"
#include <SDL_image.h>
#include <algorithm>

namespace AM
{
namespace ResourceImporter
{
SpriteSheetEditView::SpriteSheetEditView(DataModel& inDataModel)
: AUI::Window({320, 58, 1297, 1022}, "SpriteSheetEditView")
, dataModel{inDataModel}
, activeSpriteSheetID{NULL_SPRITE_SHEET_ID}
, MAX_SPRITESHEET_IMAGE_EXTENT{328, 52, 644, 732}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, checkerboardImage{{0, 0, 100, 100}, "BackgroundImage"}
, spriteSheetImage{MAX_SPRITESHEET_IMAGE_EXTENT, "SpriteSheetScrollArea"}
, descText{{24, 806, 1240, 200}, "DescText"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(checkerboardImage);
    children.push_back(spriteSheetImage);
    children.push_back(descText);

    /* Images. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditView/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    spriteSheetImage.setIsVisible(false);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Sprite Sheet");

    styleText(descText);
    descText.setText(
        "Sprite sheets are the starting point for importing graphics into the "
        "engine. Create a sheet, then add images to auto-generate Sprite and "
        "Animation objects.\nTo edit sprites, re-add your new image using the "
        "same file name.\nTo delete sprites, delete the associated Sprite in "
        "the Library.\n\nSprite objects will be auto-generated for each image "
        "added to the sheet. If the image's file name ends in \"_<number>\", "
        "e.g. \"Run_0.png\", an Animation object will also be generated. "
        "Successive numbers will add more images to the same animation.\nAll "
        "images in a given animation must be the same size.");

    // When the active sprite sheet is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&SpriteSheetEditView::onActiveLibraryItemChanged>(*this);
    dataModel.spriteModel.spriteAdded
        .connect<&SpriteSheetEditView::onSpriteAdded>(*this);
    dataModel.spriteModel.spriteRemoved
        .connect<&SpriteSheetEditView::onSpriteRemoved>(*this);
}

void SpriteSheetEditView::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a sprite sheet and return early if not.
    const EditorSpriteSheet* newActiveSpriteSheet{
        get_if<EditorSpriteSheet>(&newActiveItem)};
    if (!newActiveSpriteSheet) {
        activeSpriteSheetID = NULL_SPRITE_SHEET_ID;
        return;
    }

    activeSpriteSheetID = newActiveSpriteSheet->numericID;

    // Refresh the generated sprite sheet image.
    refreshSpriteSheetImage(*newActiveSpriteSheet);
}

void SpriteSheetEditView::onSpriteAdded(SpriteID spriteID)
{
    // If the added sprite is in the loaded sheet, refresh the image.
    if (activeSpriteSheetID) {
        const EditorSpriteSheet& spriteSheet{
            dataModel.spriteModel.getSpriteSheet(activeSpriteSheetID)};
        if (std::ranges::contains(spriteSheet.spriteIDs, spriteID)) {
            refreshSpriteSheetImage(spriteSheet);
        }
    }
}

void SpriteSheetEditView::onSpriteRemoved(SpriteID spriteID)
{
    // If the removed sprite is in the loaded sheet, refresh the image.
    if (activeSpriteSheetID) {
        const EditorSpriteSheet& spriteSheet{
            dataModel.spriteModel.getSpriteSheet(activeSpriteSheetID)};
        if (std::ranges::contains(spriteSheet.spriteIDs, spriteID)) {
            refreshSpriteSheetImage(spriteSheet);
        }
    }
}

void SpriteSheetEditView::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

void SpriteSheetEditView::refreshSpriteSheetImage(
    const EditorSpriteSheet& spriteSheet)
{
    // If there aren't any sprites in the sheet, hide the image and do nothing.
    if ((spriteSheet.textureWidth == 0) || (spriteSheet.textureHeight == 0)) {
        checkerboardImage.setIsVisible(false);
        spriteSheetImage.setIsVisible(false);
        return;
    }

    // Generate the sheet's texture and load it into the Image widget.
    SDL_Texture* spriteSheetTexture{generateSpriteSheetTexture(spriteSheet)};
    spriteSheetImage.setSimpleImage(spriteSheetTexture, "GenSpriteSheet");

    // Calc a scaled texture extent that fits inside the max image bounds 
    // without changing the texture's aspect ratio.
    SDL_Rect imageExtent{MAX_SPRITESHEET_IMAGE_EXTENT.x,
                         MAX_SPRITESHEET_IMAGE_EXTENT.y};
    int widthDiff{spriteSheet.textureWidth - MAX_SPRITESHEET_IMAGE_EXTENT.w};
    int heightDiff{spriteSheet.textureHeight - MAX_SPRITESHEET_IMAGE_EXTENT.h};
    if ((widthDiff > 0) || (heightDiff > 0)) {
        // Texture is larger than max extent in at least one direction. Scale 
        // down to fit.
        float scaleToFit{};
        if (widthDiff > heightDiff) {
            scaleToFit
                = 1
                  - (widthDiff / static_cast<float>(spriteSheet.textureWidth));
        }
        else {
            scaleToFit = 1
                         - (heightDiff
                            / static_cast<float>(spriteSheet.textureHeight));
        }

        imageExtent.w = static_cast<int>(spriteSheet.textureWidth * scaleToFit);
        imageExtent.h
            = static_cast<int>(spriteSheet.textureHeight * scaleToFit);
    }
    else {
        // Texture is smaller than max extent in both directions. No need to 
        // scale.
        imageExtent.w = spriteSheet.textureWidth;
        imageExtent.h = spriteSheet.textureHeight;
    }

    spriteSheetImage.setLogicalExtent(imageExtent);

    // Set the background's extent to match the image.
    checkerboardImage.setLogicalExtent(spriteSheetImage.getLogicalExtent());

    // Make sure everything is visible.
    checkerboardImage.setIsVisible(true);
    spriteSheetImage.setIsVisible(true);
}

SDL_Texture* SpriteSheetEditView::generateSpriteSheetTexture(
    const EditorSpriteSheet& spriteSheet)
{
    // Create an empty texture to hold the sprite sheet.
    SDL_RendererInfo info{};
    SDL_GetRendererInfo(AUI::Core::getRenderer(), &info);
    AM_ASSERT(info.num_texture_formats != 0, "No supported pixel formats.");
    SDL_Texture* spriteSheetTexture{
        SDL_CreateTexture(AUI::Core::getRenderer(), info.texture_formats[0],
                          SDL_TEXTUREACCESS_TARGET, spriteSheet.textureWidth,
                          spriteSheet.textureHeight)};

    // Set the blend mode (default is NONE, which causes black backgrounds).
    SDL_SetTextureBlendMode(spriteSheetTexture, SDL_BLENDMODE_BLEND);

    // Set the texture as the render target.
    SDL_Texture* previousRenderTarget{
        SDL_GetRenderTarget(AUI::Core::getRenderer())};
    SDL_SetRenderTarget(AUI::Core::getRenderer(), spriteSheetTexture);

    // Copy all of the sprites into the sprite sheet texture.
    std::string fullImagePath{};
    for (SpriteID spriteID : spriteSheet.spriteIDs) {
        const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};

        // Load the sprite's texture.
        fullImagePath = dataModel.getWorkingIndividualSpritesDir();
        fullImagePath += sprite.imagePath;
        SDL_Texture* spriteTexture{
            IMG_LoadTexture(AUI::Core::getRenderer(), fullImagePath.c_str())};
        if (!spriteTexture) {
            LOG_INFO("Failed to load texture: %s", fullImagePath.c_str());
            break;
        }

        // Copy the sprite into the sheet texture;
        SDL_Rect sourceRect{0, 0, sprite.textureExtent.w,
                            sprite.textureExtent.h};
        SDL_RenderCopy(AUI::Core::getRenderer(), spriteTexture, &sourceRect,
                       &(sprite.textureExtent));

        // Clean up the sprite texture.
        SDL_DestroyTexture(spriteTexture);
    }

    // Set the render target back to what it was.
    SDL_SetRenderTarget(AUI::Core::getRenderer(), previousRenderTarget);

    return spriteSheetTexture;
}

} // End namespace ResourceImporter
} // End namespace AM
