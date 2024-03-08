#include "AnimationEditStage.h"
#include "MainScreen.h"
#include "EditorSprite.h"
#include "DataModel.h"
#include "SpriteID.h"
#include "Paths.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace ResourceImporter
{
AnimationEditStage::AnimationEditStage(DataModel& inDataModel)
: AUI::Window({320, 58, 1297, 1022}, "AnimationEditStage")
, dataModel{inDataModel}
, activeAnimationID{NULL_SPRITE_ID}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, checkerboardImage{{0, 0, 100, 100}, "BackgroundImage"}
, animationImage{{0, 0, 100, 100}, "AnimationImage"}
, boundingBoxGizmo{inDataModel}
, timeline({109, 706, 1080, 48}, "AnimationTimeline")
, descText1{{24, 806, 1240, 24}, "DescText1"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(checkerboardImage);
    children.push_back(animationImage);
    children.push_back(boundingBoxGizmo);
    children.push_back(timeline);
    children.push_back(descText1);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Animation");

    styleText(descText1);
    descText1.setText("Sprites are the basic building block for graphics in "
                      "The Amalgam Engine.");

    /* Active animation and checkerboard background. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditStage/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    animationImage.setIsVisible(false);

    /* Bounding box gizmo. */
    boundingBoxGizmo.setIsVisible(false);

    // When the active animation is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&AnimationEditStage::onActiveLibraryItemChanged>(*this);
    AnimationModel& animationModel{dataModel.animationModel};
    animationModel.animationFrameCountChanged
        .connect<&AnimationEditStage::onAnimationFrameCountChanged>(*this);
    animationModel.animationFrameChanged
        .connect<&AnimationEditStage::onAnimationFrameChanged>(*this);
    animationModel.animationModelBoundsIDChanged
        .connect<&AnimationEditStage::onAnimationModelBoundsIDChanged>(*this);
    animationModel.animationCustomModelBoundsChanged
        .connect<&AnimationEditStage::onAnimationCustomModelBoundsChanged>(*this);
    animationModel.animationRemoved
        .connect<&AnimationEditStage::onAnimationRemoved>(*this);

    // When the gizmo updates the active animation's bounds, push it to the model.
    boundingBoxGizmo.setOnBoundingBoxUpdated(
        [&](const BoundingBox& updatedBounds) {
            onGizmoBoundingBoxUpdated(updatedBounds);
        });

    timeline.setOnSelectionChanged([&](const EditorSprite* sprite) {
        onTimelineSelectionChanged(sprite);
    });

    // TODO: Implement "Assign" and "Play" buttons
}

void AnimationEditStage::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is an animation and return early if not.
    const EditorAnimation* newActiveAnimation{
        get_if<EditorAnimation>(&newActiveItem)};
    if (!newActiveAnimation) {
        activeAnimationID = NULL_ANIMATION_ID;
        return;
    }

    activeAnimationID = newActiveAnimation->numericID;

    // Load this animation into the timeline.
    timeline.setActiveAnimation(*newActiveAnimation);
}

void AnimationEditStage::onAnimationFrameCountChanged(AnimationID animationID,
                                                      Uint8 newFrameCount)
{
    if (animationID == activeAnimationID) {
        timeline.setFrameCount(newFrameCount);
    }
}

void AnimationEditStage::onAnimationFrameChanged(AnimationID animationID,
                                                 Uint8 frameNumber,
                                                 const EditorSprite* newSprite)
{
    if (animationID == activeAnimationID) {
        bool hasSprite{newSprite != nullptr};
        timeline.setFrame(frameNumber, hasSprite);
    }
}

void AnimationEditStage::onAnimationModelBoundsIDChanged(
    AnimationID animationID, BoundingBoxID newModelBoundsID)
{
    // If the animation isn't active, do nothing.
    if (animationID != activeAnimationID) {
        return;
    }

    // If the animation is using a shared bounding box, disable the gizmo.
    if (newModelBoundsID) {
        boundingBoxGizmo.disable();
    }
    else {
        // The animation is using custom bounds, enable the gizmo.
        boundingBoxGizmo.enable();
    }

    // Whether it's enabled or not, the gizmo should show the correct bounds.
    const EditorAnimation& animation{dataModel.animationModel.getAnimation(animationID)};
    const BoundingBox& newModelBounds{
        animation.getModelBounds(dataModel.boundingBoxModel)};

    boundingBoxGizmo.setBoundingBox(newModelBounds);
}

void AnimationEditStage::onAnimationCustomModelBoundsChanged(
    AnimationID animationID, const BoundingBox& newCustomModelBounds)
{
    // If the animation isn't active or isn't set to custom bounds, do nothing.
    const EditorAnimation& animation{
        dataModel.animationModel.getAnimation(animationID)};
    if ((animationID != activeAnimationID) || animation.modelBoundsID) {
        return;
    }

    // Update the gizmo.
    boundingBoxGizmo.setBoundingBox(newCustomModelBounds);
}

void AnimationEditStage::onAnimationRemoved(AnimationID animationID)
{
    if (animationID == activeAnimationID) {
        activeAnimationID = NULL_ANIMATION_ID;

        // Set everything back to being invisible.
        animationImage.setIsVisible(false);
        boundingBoxGizmo.setIsVisible(false);
    }
}

void AnimationEditStage::onGizmoBoundingBoxUpdated(
    const BoundingBox& updatedBounds)
{
    if (activeAnimationID != NULL_ANIMATION_ID) {
        // If the animation isn't set to use a custom model, do nothing (should 
        // never happen since the gizmo should be disabled).
        const EditorAnimation& animation{
            dataModel.animationModel.getAnimation(activeAnimationID)};
        if (animation.modelBoundsID) {
            return;
        }

        // Update the model with the gizmo's new state.
        dataModel.animationModel.setAnimationCustomModelBounds(
            activeAnimationID, updatedBounds);
    }
}

void AnimationEditStage::onTimelineSelectionChanged(
    const EditorSprite* selectedSprite)
{
    // If the selected cell doesn't have a sprite, clear the stage and return.
    if (!selectedSprite) {
        boundingBoxGizmo.setIsVisible(false);
        return;
    }

    // Load the selected sprite image.
    std::string imagePath{dataModel.getWorkingTexturesDir()};
    imagePath += selectedSprite->parentSpriteSheetPath;
    animationImage.setSimpleImage(imagePath, selectedSprite->textureExtent);

    // TODO: When we add support for stages larger than 1x1, update this to 
    //       account for them (maybe just make the Assign button not allow 
    //       mixing stage sizes).
    // Center the sprite to the stage's X, but use a fixed Y.
    SDL_Rect centeredSpriteExtent{selectedSprite->textureExtent};
    centeredSpriteExtent.x = logicalExtent.w / 2;
    centeredSpriteExtent.x -= (centeredSpriteExtent.w / 2);
    centeredSpriteExtent.y = 212 - logicalExtent.y;
    animationImage.setLogicalExtent(centeredSpriteExtent);

    // Set the background and gizmo to the size of the sprite.
    checkerboardImage.setLogicalExtent(animationImage.getLogicalExtent());
    boundingBoxGizmo.setLogicalExtent(animationImage.getLogicalExtent());

    // Set the sprite and background to be visible.
    animationImage.setIsVisible(true);
    checkerboardImage.setIsVisible(true);

    // Set up the gizmo with the new sprite's data.
    boundingBoxGizmo.setXOffset(
        static_cast<int>(selectedSprite->textureExtent.w / 2.f));
    boundingBoxGizmo.setYOffset(selectedSprite->yOffset);
    boundingBoxGizmo.setBoundingBox(
        selectedSprite->getModelBounds(dataModel.boundingBoxModel));

    // If the gizmo isn't visible, make it visible.
    boundingBoxGizmo.setIsVisible(true);
}

void AnimationEditStage::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

} // End namespace ResourceImporter
} // End namespace AM
