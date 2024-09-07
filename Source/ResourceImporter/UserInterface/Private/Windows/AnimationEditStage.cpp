#include "AnimationEditStage.h"
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
AnimationEditStage::AnimationEditStage(DataModel& inDataModel,
                                       LibraryWindow& inLibraryWindow)
: AUI::Window({320, 58, 1297, 1022}, "AnimationEditStage")
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeAnimationID{NULL_SPRITE_ID}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, checkerboardImage{{0, 0, 100, 100}, "BackgroundImage"}
, spriteImage{{0, 0, 100, 100}, "SpriteImage"}
, boundingBoxGizmo{{0, 52, 1297, 732}, inDataModel}
, assignButton{{503, 642, 136, 46}, "Assign Sprite", "AssignButton"}
, playButton{{659, 642, 136, 46}, "Play", "PlayButton"}
, timelineScrollArea{{109, 704, 1080, 48}, "TimelineScrollArea"}
, descText{{24, 806, 1240, 100}, "DescText1"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(checkerboardImage);
    children.push_back(spriteImage);
    children.push_back(assignButton);
    children.push_back(playButton);
    children.push_back(boundingBoxGizmo);
    children.push_back(timelineScrollArea);
    children.push_back(descText);

    /* Buttons */
    assignButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 16);
    playButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 16);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Animation");

    styleText(descText);
    descText.setText("To add sprites to the timeline: Click a sprite in the "
                     "library (Ctrl+Click to select multiple), then press the "
                     "Assign Sprite button.\n\nTo move sprites within the "
                     "timeline: Right click and drag.");

    /* Active sprite and checkerboard background. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditStage/Checkerboard.png");
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

    assignButton.setOnPressed([&] { onAssignSpriteButtonPressed(); });
    playButton.setOnPressed([&] { onPlayButtonPressed(); });

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

    // When a library item is selected, update the Assign button.
    libraryWindow.selectedItemsChanged
        .connect<&AnimationEditStage::onLibrarySelectedItemsChanged>(*this);
}

void AnimationEditStage::onAssignSpriteButtonPressed()
{
    if (!activeAnimationID) {
        return;
    }

    // Assign any selected sprites to frames, starting with the current 
    // selected frame.
    const auto& selectedListItems{libraryWindow.getSelectedListItems()};
    bool spriteAssigned{false};
    int frameNumber{timeline->getSelectedFrameNumber()};
    for (const LibraryListItem* selectedItem : selectedListItems) {
        if (selectedItem->type == LibraryListItem::Type::Sprite) {
            const EditorSprite& sprite{
                dataModel.spriteModel.getSprite(selectedItem->ID)};
            
            // Assign the sprite to the frame.
            dataModel.animationModel.setAnimationFrame(activeAnimationID,
                                                       frameNumber, &sprite);
            spriteAssigned = true;
            frameNumber++;

            // If we've run out of frames, stop iterating.
            const EditorAnimation& animation{
                dataModel.animationModel.getAnimation(activeAnimationID)};
            if (frameNumber == animation.frameCount) {
                break;
            }
        }
    }

    // If no sprites were assigned, clear the frame.
    // (it isn't possible to push the button if there's no sprite selected and 
    // no sprite in the frame).
    if (!spriteAssigned) {
        dataModel.animationModel.setAnimationFrame(
            activeAnimationID, timeline->getSelectedFrameNumber(), nullptr);
    }
}

void AnimationEditStage::onPlayButtonPressed()
{
    timeline->playAnimation();
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

    // Set up the gizmo with the new animation's data.
    boundingBoxGizmo.setBoundingBox(
        newActiveAnimation->getModelBounds(dataModel.boundingBoxModel));

    // Load this animation into the timeline.
    timeline->setActiveAnimation(*newActiveAnimation);
}

void AnimationEditStage::onAnimationFrameCountChanged(AnimationID animationID,
                                                      Uint8 newFrameCount)
{
    if (animationID == activeAnimationID) {
        timeline->setFrameCount(newFrameCount);
    }
}

void AnimationEditStage::onAnimationFrameChanged(AnimationID animationID,
                                                 Uint8 frameNumber,
                                                 const EditorSprite* newSprite)
{
    if (animationID == activeAnimationID) {
        bool hasSprite{newSprite != nullptr};
        timeline->setFrame(frameNumber, hasSprite);

        // If the current frame changed, update the assign button.
        if (frameNumber == timeline->getSelectedFrameNumber()) {
            if (hasSprite) {
                assignButton.text.setText("Clear Sprite");
                assignButton.enable();
            }
            else {
                assignButton.text.setText("Assign Sprite");
                assignButton.disable();
            }
        }
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
        spriteImage.setIsVisible(false);
        checkerboardImage.setIsVisible(false);
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

void AnimationEditStage::onTimelineSelectionChanged(Uint8 selectedFrameIndex)
{
    const EditorAnimation& activeAnimation{
        dataModel.animationModel.getAnimation(activeAnimationID)};

    // If the selected frame doesn't have a sprite, clear the stage and return.
    const EditorSprite* selectedSprite{
        activeAnimation.getSpriteAtFrame(selectedFrameIndex)};
    if (!selectedSprite) {
        spriteImage.setIsVisible(false);
        assignButton.text.setText("Assign Sprite");
        assignButton.disable();
        return;
    }

    // Load the selected sprite image.
    std::string imagePath{dataModel.getWorkingTexturesDir()};
    imagePath += selectedSprite->parentSpriteSheetPath;
    spriteImage.setSimpleImage(imagePath, selectedSprite->textureExtent);

    // TODO: When we add support for stages larger than 1x1, update this to 
    //       account for them (maybe just make the Assign button not allow 
    //       mixing stage sizes).
    // Center the sprite to the stage's X, but use a fixed Y.
    SDL_Rect centeredSpriteExtent{selectedSprite->textureExtent};
    centeredSpriteExtent.x = logicalExtent.w / 2;
    centeredSpriteExtent.x -= (centeredSpriteExtent.w / 2);
    centeredSpriteExtent.y = 104;
    spriteImage.setLogicalExtent(centeredSpriteExtent);

    // Set the background and gizmo to the size of the sprite.
    checkerboardImage.setLogicalExtent(spriteImage.getLogicalExtent());
    boundingBoxGizmo.setLogicalExtent(spriteImage.getLogicalExtent());

    // Set the sprite and background to be visible.
    spriteImage.setIsVisible(true);
    checkerboardImage.setIsVisible(true);

    // Set up the gizmo with the new sprite's data.
    boundingBoxGizmo.setStageOrigin(selectedSprite->stageOrigin);

    // If the gizmo isn't visible, make it visible.
    boundingBoxGizmo.setIsVisible(true);

    // Let the user clear the sprite.
    assignButton.text.setText("Clear Sprite");
    assignButton.enable();
}

void AnimationEditStage::onTimelineSpriteMoved(Uint8 oldFrameIndex,
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

void AnimationEditStage::onLibrarySelectedItemsChanged(
    const std::vector<LibraryListItem*>& selectedItems)
{
    // If there's no active animation, do nothing.
    if (!activeAnimationID) {
        return;
    }

    // If a sprite is selected, allow the user to assign it.
    if ((selectedItems.size() > 0)
        && (selectedItems[0]->type == LibraryListItem::Type::Sprite)) {
        assignButton.text.setText("Assign Sprite");
        assignButton.enable();
    }
    // If a sprite isn't selected but we're displaying an image, allow the 
    // user to clear it.
    else if (spriteImage.getIsVisible()) {
        assignButton.text.setText("Clear Sprite");
        assignButton.enable();
    }
    else {
        // No selection and no image. Disable the button.
        assignButton.text.setText("Assign Sprite");
        assignButton.disable();
    }
}

void AnimationEditStage::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

} // End namespace ResourceImporter
} // End namespace AM
