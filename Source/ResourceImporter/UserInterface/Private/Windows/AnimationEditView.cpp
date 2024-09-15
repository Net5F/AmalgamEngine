#include "AnimationEditView.h"
#include "DataModel.h"
#include "LibraryWindow.h"
#include "AnimationTimeline.h"
#include "SpriteID.h"
#include "Paths.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace ResourceImporter
{
AnimationEditView::AnimationEditView(DataModel& inDataModel,
                                     LibraryWindow& inLibraryWindow)
: AUI::Window({320, 58, 1297, 1022}, "AnimationEditView")
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeAnimationID{NULL_SPRITE_ID}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, checkerboardImage{{0, 0, 100, 100}, "BackgroundImage"}
, stageGraphic{logicalExtent}
, spriteImage{{0, 0, 100, 100}, "SpriteImage"}
, boundingBoxGizmo{{0, 52, 1297, 732}, inDataModel}
, timelineScrollArea{{109, 796, 1080, 48}, "TimelineScrollArea"}
, descText{{24, 898, 1240, 100}, "DescText"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(checkerboardImage);
    children.push_back(stageGraphic);
    children.push_back(spriteImage);
    children.push_back(boundingBoxGizmo);
    children.push_back(timelineScrollArea);
    children.push_back(descText);

    // Flag ourselves as focusable, so we can receive keyboard events.
    isFocusable = true;

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Animation");

    styleText(descText);
    descText.setText("To play the animation: press the space bar.\n\nTo move "
                     "sprites within the timeline: right click and drag.");

    /* Active sprite and checkerboard background. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditView/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    spriteImage.setIsVisible(false);

    /* Bounding box gizmo. */
    boundingBoxGizmo.setIsVisible(false);

    /* Timeline */
    SDL_Rect timelineExtent{timelineScrollArea.getLogicalExtent()};
    timelineScrollArea.content = std::make_unique<AnimationTimeline>(
        SDL_Rect{0, 0, timelineExtent.w, timelineExtent.h},
        "AnimationTimeline");
    timelineScrollArea.setScrollOrientation(AUI::Orientation::Horizontal);
    timeline
        = static_cast<AnimationTimeline*>(timelineScrollArea.content.get());

    // When the active animation is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&AnimationEditView::onActiveLibraryItemChanged>(*this);
    AnimationModel& animationModel{dataModel.animationModel};
    animationModel.animationFrameCountChanged
        .connect<&AnimationEditView::onAnimationFrameCountChanged>(*this);
    animationModel.animationFrameChanged
        .connect<&AnimationEditView::onAnimationFrameChanged>(*this);
    animationModel.animationModelBoundsIDChanged
        .connect<&AnimationEditView::onAnimationModelBoundsIDChanged>(*this);
    animationModel.animationCustomModelBoundsChanged
        .connect<&AnimationEditView::onAnimationCustomModelBoundsChanged>(*this);
    animationModel.animationRemoved
        .connect<&AnimationEditView::onAnimationRemoved>(*this);

    // When the gizmo updates the active animation's bounds, push it to the model.
    boundingBoxGizmo.setOnBoundingBoxUpdated(
        [&](const BoundingBox& updatedBounds) {
            onGizmoBoundingBoxUpdated(updatedBounds);
        });

    timeline->setOnSelectionChanged([&](Uint8 selectedFrameIndex) {
        onTimelineSelectionChanged(selectedFrameIndex);
    });
    timeline->setOnSpriteMoved([&](Uint8 oldFrameIndex, Uint8 newFrameIndex) {
        onTimelineSpriteMoved(oldFrameIndex, newFrameIndex);
    });
}

AUI::EventResult AnimationEditView::onKeyDown(SDL_Keycode keyCode)
{
    // If the space bar was pressed, play the animation.
    if (keyCode == SDLK_SPACE) {
        timeline->playAnimation();

        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}};
}

void AnimationEditView::onActiveLibraryItemChanged(
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

    // Note: All sprites in an animation must have the same textureExtent and 
    //       stageOrigin, so we can calculate everything once here instead of 
    //       needing to re-calc whenever the sprite changes.

    // Set up the gizmo with the new animation's size and data.
    AM_ASSERT(newActiveAnimation->frames.size() > 0,
              "Animation must always have at least 1 frame.");
    const EditorSprite& firstSprite{newActiveAnimation->frames[0].sprite.get()};
    boundingBoxGizmo.setSpriteImageSize(firstSprite.textureExtent.w,
                                        firstSprite.textureExtent.h);
    boundingBoxGizmo.setStageOrigin(firstSprite.stageOrigin);
    boundingBoxGizmo.setBoundingBox(
        newActiveAnimation->getModelBounds(dataModel.boundingBoxModel));

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
    stageGraphic.updateStage(firstSprite.textureExtent,
                             firstSprite.stageOrigin,
                             {(gizmoClippedExtent.x + actualSpriteExtent.x),
                              (gizmoClippedExtent.y + actualSpriteExtent.y)});

    // Make sure everything is visible.
    boundingBoxGizmo.setIsVisible(true);
    checkerboardImage.setIsVisible(true);
    stageGraphic.setIsVisible(true);
    spriteImage.setIsVisible(true);

    // Load this animation into the timeline.
    timeline->setActiveAnimation(*newActiveAnimation);
}

void AnimationEditView::onAnimationFrameCountChanged(AnimationID animationID,
                                                      Uint8 newFrameCount)
{
    if (animationID == activeAnimationID) {
        timeline->setFrameCount(newFrameCount);
    }
}

void AnimationEditView::onAnimationFrameChanged(AnimationID animationID,
                                                 Uint8 frameNumber,
                                                 const EditorSprite* newSprite)
{
    if (animationID == activeAnimationID) {
        bool hasSprite{newSprite != nullptr};
        timeline->setFrame(frameNumber, hasSprite);
    }
}

void AnimationEditView::onAnimationModelBoundsIDChanged(
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

void AnimationEditView::onAnimationCustomModelBoundsChanged(
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

void AnimationEditView::onAnimationRemoved(AnimationID animationID)
{
    if (animationID == activeAnimationID) {
        activeAnimationID = NULL_ANIMATION_ID;

        // Set everything back to being invisible.
        spriteImage.setIsVisible(false);
        checkerboardImage.setIsVisible(false);
        boundingBoxGizmo.setIsVisible(false);
    }
}

void AnimationEditView::onGizmoBoundingBoxUpdated(
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

void AnimationEditView::onTimelineSelectionChanged(Uint8 selectedFrameIndex)
{
    const EditorAnimation& activeAnimation{
        dataModel.animationModel.getAnimation(activeAnimationID)};

    // If the selected frame doesn't have a sprite, clear the stage and return.
    const EditorSprite* selectedSprite{
        activeAnimation.getSpriteAtFrame(selectedFrameIndex)};
    if (!selectedSprite) {
        spriteImage.setIsVisible(false);
        return;
    }

    // Load the selected sprite image.
    std::string imagePath{dataModel.getWorkingTexturesDir()};
    imagePath += selectedSprite->parentSpriteSheetPath;
    spriteImage.setSimpleImage(imagePath, selectedSprite->textureExtent);

}

void AnimationEditView::onTimelineSpriteMoved(Uint8 oldFrameIndex,
                                               Uint8 newFrameIndex)
{
    AnimationModel& animationModel{dataModel.animationModel};
    const EditorAnimation& activeAnimation{
        animationModel.getAnimation(activeAnimationID)};

    // Swap the sprites (or clear if swapping with empty).
    const EditorSprite* oldSprite{
        activeAnimation.getSpriteAtFrame(oldFrameIndex)};
    const EditorSprite* newSprite{
        activeAnimation.getSpriteAtFrame(newFrameIndex)};
    animationModel.setAnimationFrame(activeAnimationID, oldFrameIndex,
                                     newSprite);
    animationModel.setAnimationFrame(activeAnimationID, newFrameIndex,
                                     oldSprite);
}

void AnimationEditView::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

} // End namespace ResourceImporter
} // End namespace AM
