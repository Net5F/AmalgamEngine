#pragma once

#include "LibraryItemData.h"
#include "BoundingBoxGizmo.h"
#include "MainButton.h"
#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Text.h"
#include "AUI/Image.h"
#include "AUI/ScrollArea.h"

namespace AM
{
namespace ResourceImporter
{
class DataModel;
class LibraryWindow;
class AnimationTimeline;
class LibraryListItem;

/**
 * The center stage shown when the user loads an animation from the Library.
 * Allows the user to add sprites to the animation, set fps and frame count, 
 * and edit its bounding box.
 */
class AnimationEditStage : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    AnimationEditStage(DataModel& inDataModel, LibraryWindow& inLibraryWindow);

private:
    /**
     * Adds or removes sprites from the currently selected frame, depending 
     * on whether a sprite is selected in the library.
     */
    void onAssignSpriteButtonPressed();

    /**
     * If the new active item is an animation, loads it's data onto this stage.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * If the changed animation is currently active, updates this stage to 
     * reflect the new data.
     */
    void onAnimationFrameCountChanged(AnimationID animationID,
                                      Uint8 newFrameCount);
    void onAnimationFrameChanged(AnimationID animationID, Uint8 frameNumber,
                                 const EditorSprite* newSprite);
    void onAnimationModelBoundsIDChanged(AnimationID animationID,
                                         BoundingBoxID newModelBoundsID);
    void onAnimationCustomModelBoundsChanged(
        AnimationID animationID, const BoundingBox& newCustomModelBounds);

    /**
     * (If active animation was removed) Sets activeAnimation to invalid and 
     * returns the stage to its default state.
     */
    void onAnimationRemoved(AnimationID animationID);

    /**
     * Pushes the gizmo's updated bounding box to the model.
     */
    void onGizmoBoundingBoxUpdated(const BoundingBox& updatedBounds);

    /**
     * Displays the timeline's new selected sprite.
     */
    void onTimelineSelectionChanged(const EditorSprite* selectedSprite);

    /**
     * Tells the model to move the sprite.
     */
    void onTimelineSpriteMoved(Uint8 oldFrameIndex, Uint8 newFrameIndex,
                               const EditorSprite* movedSprite);

    /**
     * Updates assignButton to show whether the selection is assign-able.
     */
    void onLibrarySelectedItemsChanged(
        const std::vector<LibraryListItem*>& selectedItems);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /** Used to get the current working dir when displaying the animation. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    LibraryWindow& libraryWindow;

    /** The active animation's ID. */
    AnimationID activeAnimationID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;

    /** Checkerboard image, tiled as the background for the current frame's 
        sprite. */
    AUI::Image checkerboardImage;

    /** The sprite for the currently selected frame. */
    AUI::Image spriteImage;

    /** The gizmo for editing the animation's bounding box. */
    BoundingBoxGizmo boundingBoxGizmo;

    /** The button for assigning sprites to the curretn selected frame. */
    MainButton assignButton;

    /** The button for playing the animation. */
    MainButton playButton;

    /** Holds the AnimationTimeline so it can be scrolled horizontally. */
    AUI::ScrollArea timelineScrollArea;

    /** The timeline for editing the sprites in the animation. */
    AnimationTimeline* timeline;

    AUI::Text descText1;
};

} // End namespace ResourceImporter
} // End namespace AM
