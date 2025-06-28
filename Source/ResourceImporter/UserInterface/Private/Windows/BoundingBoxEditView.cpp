#include "BoundingBoxEditView.h"
#include "DataModel.h"
#include "LibraryWindow.h"
#include "Paths.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace ResourceImporter
{
BoundingBoxEditView::BoundingBoxEditView(DataModel& inDataModel,
                                           LibraryWindow& inLibraryWindow)
: AUI::Window({320, 58, 1297, 1022}, "BoundingBoxEditView")
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeBoundingBoxID{NULL_BOUNDING_BOX_ID}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, modifyText{{0, 58, 1297, 24}, "ModifyText"}
, checkerboardImage{{0, 0, 100, 100}, "BackgroundImage"}
, stageGraphic{logicalExtent}
, spriteImage{{0, 0, 256, 512}, "SpriteImage"}
, boundingBoxGizmo{{0, 52, 1297, 732}}
, descText{{24, 806, 1240, 144}, "DescText"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(modifyText);
    children.push_back(checkerboardImage);
    children.push_back(stageGraphic);
    children.push_back(spriteImage);
    children.push_back(boundingBoxGizmo);
    children.push_back(descText);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Bounding Box");

    styleText(descText);
    descText.setText(
        "Bounding boxes are used by Sprites and Animations for collision and "
        "to make them clickable in build mode.");

    styleText(modifyText);
    modifyText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    modifyText.setText("To modify: open a Sprite or Animation in the Library "
                       "window, set a custom box, then choose \"Save as\".");

    /* Active sprite and checkerboard background. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditView/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    spriteImage.setIsVisible(false);

    /* Bounding box gizmo. */
    boundingBoxGizmo.setIsVisible(false);

    // When the active bounding box is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&BoundingBoxEditView::onActiveLibraryItemChanged>(*this);
    dataModel.boundingBoxModel.boundingBoxBoundsChanged
        .connect<&BoundingBoxEditView::onBoundingBoxBoundsChanged>(*this);
    dataModel.boundingBoxModel.boundingBoxRemoved
        .connect<&BoundingBoxEditView::onBoundingBoxRemoved>(*this);

    // When the gizmo updates the active sprite's bounds, push it to the model.
    boundingBoxGizmo.setOnBoundingBoxUpdated(
        [&](const BoundingBox& updatedBounds) {
            onGizmoBoundingBoxUpdated(updatedBounds);
        });
}

void BoundingBoxEditView::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a bounding box and return early if not.
    const EditorBoundingBox* newActiveBoundingBox{
        get_if<EditorBoundingBox>(&newActiveItem)};
    if (!newActiveBoundingBox) {
        activeBoundingBoxID = NULL_BOUNDING_BOX_ID;
        return;
    }

    activeBoundingBoxID = newActiveBoundingBox->numericID;

    // Set up the gizmo with the default sprite's size and data.
    const SDL_Rect defaultSpriteExtent{0, 0, 256, 512};
    const SDL_Point defaultSpriteOrigin{128, 374};
    boundingBoxGizmo.setSpriteImageSize(defaultSpriteExtent.w,
                                        defaultSpriteExtent.h);
    boundingBoxGizmo.setStageOrigin(defaultSpriteOrigin);
    boundingBoxGizmo.setBoundingBox(newActiveBoundingBox->modelBounds);

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
    stageGraphic.updateStage(defaultSpriteExtent, defaultSpriteOrigin,
                             {(gizmoClippedExtent.x + actualSpriteExtent.x),
                              (gizmoClippedExtent.y + actualSpriteExtent.y)});

    // Make sure everything is visible.
    boundingBoxGizmo.setIsVisible(true);
    checkerboardImage.setIsVisible(true);
    stageGraphic.setIsVisible(true);
    spriteImage.setIsVisible(true);
}

void BoundingBoxEditView::onBoundingBoxBoundsChanged(
    BoundingBoxID boundingBoxID, const BoundingBox& newBounds)
{
    // If the box isn't active, do nothing.
    if (boundingBoxID != activeBoundingBoxID) {
        return;
    }

    // Update the gizmo to reflect the new bounds.
    const EditorBoundingBox& boundingBox{
        dataModel.boundingBoxModel.getBoundingBox(boundingBoxID)};
    boundingBoxGizmo.setBoundingBox(boundingBox.modelBounds);
}

void BoundingBoxEditView::onBoundingBoxRemoved(BoundingBoxID boundingBoxID)
{
    // If the active box was deleted, hide this window.
    if (boundingBoxID == activeBoundingBoxID) {
        activeBoundingBoxID = NULL_BOUNDING_BOX_ID;
        setIsVisible(false);
    }
}

void BoundingBoxEditView::onGizmoBoundingBoxUpdated(
    const BoundingBox& updatedBounds)
{
    if (activeBoundingBoxID) {
        // Update the model with the gizmo's new state.
        dataModel.boundingBoxModel.setBoundingBoxBounds(activeBoundingBoxID,
                                                        updatedBounds);
    }
}

void BoundingBoxEditView::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

} // End namespace ResourceImporter
} // End namespace AM
