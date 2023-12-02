#include "SpriteEditStage.h"
#include "MainScreen.h"
#include "EditorSprite.h"
#include "DataModel.h"
#include "NullSpriteID.h"
#include "Paths.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace SpriteEditor
{
SpriteEditStage::SpriteEditStage(DataModel& inDataModel)
: AUI::Window({320, 58, 1297, 1022}, "SpriteEditStage")
, dataModel{inDataModel}
, activeSpriteID{NULL_SPRITE_ID}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, checkerboardImage{{0, 0, 100, 100}, "BackgroundImage"}
, spriteImage{{0, 0, 100, 100}, "SpriteImage"}
, boundingBoxGizmo{inDataModel}
, descText1{{24, 806, 1240, 24}, "DescText1"}
, descText2{{24, 846, 1240, 24}, "DescText2"}
, descText3{{24, 886, 1240, 24}, "DescText3"}
, descText4{{24, 926, 1240, 24}, "DescText4"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(checkerboardImage);
    children.push_back(spriteImage);
    children.push_back(boundingBoxGizmo);
    children.push_back(descText1);
    children.push_back(descText2);
    children.push_back(descText3);
    children.push_back(descText4);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Sprite");

    styleText(descText1);
    descText1.setText("Sprites are the basic building block for graphics in "
                      "The Amalgam Engine.");
    styleText(descText2);
    descText2.setText(
        "The bounding box that you set for this sprite will be used for render "
        "sorting, mouse hit detection, and (if enabled) collision.");
    styleText(descText3);
    descText3.setText(
        "Sprites must be added to a Sprite Set to be used in the engine.");
    styleText(descText4);
    descText4.setText("Sprite Sets come in various types: Floor, Floor "
                      "Covering, Wall, and Object.");

    /* Active sprite and checkerboard background. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditStage/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    spriteImage.setIsVisible(false);

    /* Bounding box gizmo. */
    boundingBoxGizmo.setIsVisible(false);

    // When the active sprite is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&SpriteEditStage::onActiveLibraryItemChanged>(*this);
    dataModel.spriteModel.spriteRemoved
        .connect<&SpriteEditStage::onSpriteRemoved>(*this);
}

void SpriteEditStage::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a sprite and return early if not.
    const EditorSprite* newActiveSprite{
        std::get_if<EditorSprite>(&newActiveItem)};
    if (newActiveSprite == nullptr) {
        activeSpriteID = NULL_SPRITE_ID;
        return;
    }

    activeSpriteID = newActiveSprite->numericID;

    // Load the sprite's image.
    std::string imagePath{dataModel.getWorkingTexturesDir()};
    imagePath += newActiveSprite->parentSpriteSheetPath;
    spriteImage.setSimpleImage(imagePath, newActiveSprite->textureExtent);

    // Center the sprite to the stage's X, but use a fixed Y.
    SDL_Rect centeredSpriteExtent{newActiveSprite->textureExtent};
    centeredSpriteExtent.x = logicalExtent.w / 2;
    centeredSpriteExtent.x -= (centeredSpriteExtent.w / 2);
    centeredSpriteExtent.y = AUI::ScalingHelpers::logicalToActual(212);

    // Size the sprite image to the sprite extent size.
    spriteImage.setLogicalExtent(centeredSpriteExtent);

    // Set the background and gizmo to the size of the sprite.
    checkerboardImage.setLogicalExtent(spriteImage.getLogicalExtent());
    boundingBoxGizmo.setLogicalExtent(spriteImage.getLogicalExtent());

    // Set the sprite and background to be visible.
    checkerboardImage.setIsVisible(true);
    spriteImage.setIsVisible(true);

    // If the gizmo isn't visible, make it visible.
    boundingBoxGizmo.setIsVisible(true);
}

void SpriteEditStage::onSpriteRemoved(int spriteID)
{
    if (spriteID == activeSpriteID) {
        activeSpriteID = NULL_SPRITE_ID;

        // Set everything back to being invisible.
        checkerboardImage.setIsVisible(false);
        spriteImage.setIsVisible(false);
        boundingBoxGizmo.setIsVisible(false);
    }
}

void SpriteEditStage::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

} // End namespace SpriteEditor
} // End namespace AM
