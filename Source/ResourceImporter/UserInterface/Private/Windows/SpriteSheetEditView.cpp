#include "SpriteSheetEditView.h"
#include "MainScreen.h"
#include "EditorSpriteSheet.h"
#include "DataModel.h"
#include "Paths.h"
#include "AUI/Image.h"
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
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, spriteSheetScrollArea{{328, 52, 1297, 1022}, "SpriteSheetScrollArea"}
, spriteSheetImage{nullptr}
, descText{{24, 806, 1240, 200}, "DescText"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(spriteSheetScrollArea);
    children.push_back(descText);

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

    /* Sprite sheet image. */
    spriteSheetScrollArea.setIsVisible(false);
    SDL_Rect spriteSheetExtent{spriteSheetScrollArea.getLogicalExtent()};
    spriteSheetScrollArea.content = std::make_unique<AUI::Image>(
        SDL_Rect{0, 0, spriteSheetExtent.w, spriteSheetExtent.h},
        "SpriteSheetImage");
    spriteSheetImage
        = static_cast<AUI::Image*>(spriteSheetScrollArea.content.get());

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

    // Generate the sheet's texture and load it into the Image widget.
    SDL_Texture* spriteSheetTexture{
        generateSpriteSheetTexture(*newActiveSpriteSheet)};
    spriteSheetImage->setSimpleImage(spriteSheetTexture, "GenSpriteSheet");

    // Make sure everything is visible.
    spriteSheetImage->setIsVisible(true);
}

void SpriteSheetEditView::onSpriteAdded(SpriteID spriteID)
{
    // If the added sprite is in the loaded sheet, regen the texture
    if (activeSpriteSheetID) {
        const EditorSpriteSheet& spriteSheet{
            dataModel.spriteModel.getSpriteSheet(activeSpriteSheetID)};
        if (std::ranges::contains(spriteSheet.spriteIDs, spriteID)) {
            SDL_Texture* spriteSheetTexture{
                generateSpriteSheetTexture(spriteSheet)};
            spriteSheetImage->setSimpleImage(spriteSheetTexture,
                                            "GenSpriteSheet");
        }
    }
}

void SpriteSheetEditView::onSpriteRemoved(SpriteID spriteID)
{
    // If the removed sprite is in the loaded sheet, regen the texture
    if (activeSpriteSheetID) {
        const EditorSpriteSheet& spriteSheet{
            dataModel.spriteModel.getSpriteSheet(activeSpriteSheetID)};
        if (std::ranges::contains(spriteSheet.spriteIDs, spriteID)) {
            SDL_Texture* spriteSheetTexture{
                generateSpriteSheetTexture(spriteSheet)};
            spriteSheetImage->setSimpleImage(spriteSheetTexture,
                                            "GenSpriteSheet");
        }
    }
}

void SpriteSheetEditView::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

SDL_Texture* SpriteSheetEditView::generateSpriteSheetTexture(
    const EditorSpriteSheet& spriteSheet)
{
    // TODO: Figure out what this pixel format should be
    // Create an empty texture to hold the sprite sheet.
    SDL_Texture* spriteSheetTexture{
        SDL_CreateTexture(AUI::Core::getRenderer(), SDL_PIXELFORMAT_BGRA8888,
                          SDL_TEXTUREACCESS_TARGET, spriteSheet.textureWidth,
                          spriteSheet.textureHeight)};

    // Copy all of the sprites into the sprite sheet texture.
    std::string fullImagePath{};
    for (SpriteID spriteID : spriteSheet.spriteIDs) {
        const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};

        // Load the sprite's texture.
        fullImagePath = dataModel.getWorkingTexturesDir();
        fullImagePath += sprite.imagePath;
        SDL_Texture* spriteTexture{
            IMG_LoadTexture(AUI::Core::getRenderer(), fullImagePath.c_str())};
        if (spriteTexture) {
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

    return spriteSheetTexture;
}

} // End namespace ResourceImporter
} // End namespace AM
