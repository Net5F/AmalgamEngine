#include "SpriteEditView.h"
#include "MainScreen.h"
#include "EditorSprite.h"
#include "DataModel.h"
#include "SpriteID.h"
#include "Paths.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace ResourceImporter
{
SpriteEditView::SpriteEditView(DataModel& inDataModel)
: AUI::Window({320, 58, 1297, 1022}, "SpriteEditView")
, dataModel{inDataModel}
, activeSpriteID{NULL_SPRITE_ID}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, checkerboardImage{{0, 0, 100, 100}, "BackgroundImage"}
, stageGraphic{logicalExtent}
, spriteImage{{0, 0, 100, 100}, "SpriteImage"}
, boundingBoxGizmo{{0, 52, 1297, 732}, inDataModel}
, descText{{24, 806, 1240, 200}, "DescText"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(checkerboardImage);
    children.push_back(stageGraphic);
    children.push_back(spriteImage);
    children.push_back(boundingBoxGizmo);
    children.push_back(descText);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Sprite");

    styleText(descText);
    descText.setText(
        "Sprites are the basic building block for graphics in The Amalgam "
        "Engine.\n\nThe bounding box that you set for this sprite will be used "
        "for render sorting, mouse hit detection, and (if enabled) "
        "collision.\n\nSprites must be added to a Sprite Set to be used in the "
        "engine.\n\nSprite Sets come in various types: Terrain, Floor, Wall, "
        "Object, and Entity.");

    /* Images and graphics. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditView/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    stageGraphic.setIsVisible(false);
    spriteImage.setIsVisible(false);

    /* Bounding box gizmo. */
    boundingBoxGizmo.setIsVisible(false);

    // When the active sprite is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&SpriteEditView::onActiveLibraryItemChanged>(*this);
    dataModel.spriteModel.spriteModelBoundsIDChanged
        .connect<&SpriteEditView::onSpriteModelBoundsIDChanged>(*this);
    dataModel.spriteModel.spriteCustomModelBoundsChanged
        .connect<&SpriteEditView::onSpriteCustomModelBoundsChanged>(*this);
    dataModel.spriteModel.spriteStageOriginChanged
        .connect<&SpriteEditView::onSpriteStageOriginChanged>(*this);
    dataModel.spriteModel.spriteRemoved
        .connect<&SpriteEditView::onSpriteRemoved>(*this);

    // When the gizmo updates the active sprite's bounds, push it to the model.
    boundingBoxGizmo.setOnBoundingBoxUpdated(
        [&](const BoundingBox& updatedBounds) {
            onGizmoBoundingBoxUpdated(updatedBounds);
        });
}

void SpriteEditView::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a sprite and return early if not.
    const EditorSprite* newActiveSprite{get_if<EditorSprite>(&newActiveItem)};
    if (!newActiveSprite) {
        activeSpriteID = NULL_SPRITE_ID;
        return;
    }

    activeSpriteID = newActiveSprite->numericID;

    // Load the sprite's image.
    std::string fullImagePath{dataModel.getWorkingIndividualSpritesDir()};
    fullImagePath += newActiveSprite->imagePath;
    spriteImage.setSimpleImage(fullImagePath,
                               {0, 0, newActiveSprite->textureExtent.w,
                                newActiveSprite->textureExtent.h});

    // Set up the gizmo with the new sprite's size and data.
    // Note: The sprite's native size is used as the logical size.
    boundingBoxGizmo.setSpriteImageSize(newActiveSprite->textureExtent.w,
                                        newActiveSprite->textureExtent.h);
    boundingBoxGizmo.setStageOrigin(newActiveSprite->stageOrigin);
    boundingBoxGizmo.setBoundingBox(
        newActiveSprite->getModelBounds(dataModel.boundingBoxModel));

    // If the sprite is using a shared bounding box, disable the gizmo.
    if (newActiveSprite->modelBoundsID) {
        boundingBoxGizmo.disable();
    }
    else {
        // The sprite is using custom bounds, enable the gizmo.
        boundingBoxGizmo.enable();
    }

    // Use the gizmo's centered sprite extent to set the background and sprite
    // extents.
    SDL_Rect logicalSpriteExtent{
        boundingBoxGizmo.getLogicalCenteredSpriteExtent()};
    logicalSpriteExtent.x += boundingBoxGizmo.getLogicalExtent().x;
    logicalSpriteExtent.y += boundingBoxGizmo.getLogicalExtent().y;
    checkerboardImage.setLogicalExtent(logicalSpriteExtent);
    spriteImage.setLogicalExtent(logicalSpriteExtent);

    // Set up the stage graphic.
    const SDL_Rect& gizmoClippedExtent{boundingBoxGizmo.getClippedExtent()};
    SDL_Rect actualSpriteExtent{AUI::ScalingHelpers::logicalToActual(
        boundingBoxGizmo.getLogicalCenteredSpriteExtent())};
    stageGraphic.updateStage(newActiveSprite->textureExtent,
                             newActiveSprite->stageOrigin,
                             {(gizmoClippedExtent.x + actualSpriteExtent.x),
                              (gizmoClippedExtent.y + actualSpriteExtent.y)});

    // Make sure everything is visible.
    boundingBoxGizmo.setIsVisible(true);
    checkerboardImage.setIsVisible(true);
    stageGraphic.setIsVisible(true);
    spriteImage.setIsVisible(true);
}

void SpriteEditView::onSpriteModelBoundsIDChanged(
    SpriteID spriteID, BoundingBoxID newModelBoundsID)
{
    // If the sprite isn't active, do nothing.
    if (spriteID != activeSpriteID) {
        return;
    }

    // If the sprite is using a shared bounding box, disable the gizmo.
    if (newModelBoundsID) {
        boundingBoxGizmo.disable();
    }
    else {
        // The sprite is using custom bounds, enable the gizmo.
        boundingBoxGizmo.enable();
    }

    // Whether it's enabled or not, the gizmo should show the correct bounds.
    const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};
    const BoundingBox& newModelBounds{
        sprite.getModelBounds(dataModel.boundingBoxModel)};

    boundingBoxGizmo.setBoundingBox(newModelBounds);
}

void SpriteEditView::onSpriteCustomModelBoundsChanged(
    SpriteID spriteID, const BoundingBox& newCustomModelBounds)
{
    // If the sprite isn't active or isn't set to custom bounds, do nothing.
    const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};
    if ((spriteID != activeSpriteID) || sprite.modelBoundsID) {
        return;
    }

    // Update the gizmo.
    boundingBoxGizmo.setBoundingBox(newCustomModelBounds);
}

void SpriteEditView::onSpriteStageOriginChanged(SpriteID spriteID,
                                                const SDL_Point& newStageOrigin)
{
    // If the sprite isn't active or isn't set to custom bounds, do nothing.
    const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};
    if ((spriteID != activeSpriteID) || sprite.modelBoundsID) {
        return;
    }

    // Update up the stage graphic.
    const SDL_Rect& gizmoClippedExtent{boundingBoxGizmo.getClippedExtent()};
    SDL_Rect actualSpriteExtent{AUI::ScalingHelpers::logicalToActual(
        boundingBoxGizmo.getLogicalCenteredSpriteExtent())};
    stageGraphic.updateStage(sprite.textureExtent, sprite.stageOrigin,
                             {(gizmoClippedExtent.x + actualSpriteExtent.x),
                              (gizmoClippedExtent.y + actualSpriteExtent.y)});

    // Update the gizmo.
    boundingBoxGizmo.setStageOrigin(sprite.stageOrigin);
}

void SpriteEditView::onSpriteRemoved(SpriteID spriteID)
{
    if (spriteID == activeSpriteID) {
        activeSpriteID = NULL_SPRITE_ID;

        // Set everything back to being invisible.
        checkerboardImage.setIsVisible(false);
        spriteImage.setIsVisible(false);
        boundingBoxGizmo.setIsVisible(false);
    }
}

void SpriteEditView::onGizmoBoundingBoxUpdated(
    const BoundingBox& updatedBounds)
{
    if (activeSpriteID != NULL_SPRITE_ID) {
        // If the sprite isn't set to use a custom model, do nothing (should
        // never happen since the gizmo should be disabled).
        const EditorSprite& sprite{
            dataModel.spriteModel.getSprite(activeSpriteID)};
        if (sprite.modelBoundsID) {
            return;
        }

        // Update the model with the gizmo's new state.
        dataModel.spriteModel.setSpriteCustomModelBounds(activeSpriteID,
                                                         updatedBounds);
    }
}

void SpriteEditView::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

} // End namespace ResourceImporter
} // End namespace AM
