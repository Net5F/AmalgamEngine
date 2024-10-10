#pragma once

#include "LibraryItemData.h"
#include "StageGraphic.h"
#include "BoundingBoxGizmo.h"
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
class AnimationEditView : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    AnimationEditView(DataModel& inDataModel, LibraryWindow& inLibraryWindow);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    AUI::EventResult onKeyDown(SDL_Keycode keyCode) override;

private:
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
    void onTimelineSelectionChanged(Uint8 selectedFrameNumber);

    /**
     * Tells the model to move the sprite.
     */
    void onTimelineSpriteMoved(Uint8 oldFrameNumber, Uint8 newFrameNumber);

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

    /** The transparent graphic that shows the stage bounds. */
    StageGraphic stageGraphic;

    /** The sprite for the currently selected frame. */
    AUI::Image spriteImage;

    /** The gizmo for editing the animation's bounding box. */
    BoundingBoxGizmo boundingBoxGizmo;

    /** Holds the AnimationTimeline so it can be scrolled horizontally. */
    AUI::ScrollArea timelineScrollArea;

    /** The timeline for editing the sprites in the animation. */
    AnimationTimeline* timeline;

    AUI::Text descText;
};

} // End namespace ResourceImporter
} // End namespace AM
