#pragma once

#include "AnimationElementsWindow.h"
#include "LibraryItemData.h"
#include "StageGraphic.h"
#include "BoundingBoxGizmo.h"
#include "PointGizmo.h"
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
    AnimationEditView(DataModel& inDataModel, LibraryWindow& inLibraryWindow,
                      AnimationElementsWindow& inAnimationElementsWindow);

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
    void onAnimationLoopStartFrameChanged(AnimationID animationID,
                                          Uint8 newLoopStartFrame);
    void onAnimationFrameChanged(AnimationID animationID, Uint8 frameNumber,
                                 const EditorSprite* newSprite);
    void onAnimationModelBoundsIDChanged(AnimationID animationID,
                                         BoundingBoxID newModelBoundsID);
    void onAnimationCustomModelBoundsChanged(
        AnimationID animationID, const BoundingBox& newCustomModelBounds);
    void onAnimationEntityAlignmentAnchorChanged(
        AnimationID animationID,
        const std::optional<Vector3>& newEntityAlignmentAnchor);

    /**
     * (If active animation was removed) Hides this view.
     */
    void onAnimationRemoved(AnimationID animationID);

    /**
     * Pushes the gizmo's updated bounding box to the model.
     */
    void onGizmoBoundingBoxUpdated(const BoundingBox& updatedBounds);

    /**
     * Pushes the gizmo's updated entity alignment anchor to the model.
     */
    void onGizmoEntityAlignmentAnchorUpdated(
        const Vector3& updatedEntityAlignmentAnchor);

    /**
     * Displays the timeline's new selected sprite.
     */
    void onTimelineSelectionChanged(Uint8 selectedFrameNumber);

    /**
     * Tells the model to update the animation.
     */
    void onTimelineLoopStartFrameChanged(Uint8 newLoopStartFrame);

    /**
     * Tells the model to move the sprite.
     */
    void onTimelineSpriteMoved(Uint8 oldFrameNumber, Uint8 newFrameNumber);

    /**
     * Enables the appropriate gizmo for the selected element.
     */
    void onElementSelected(AnimationElementsWindow::ElementType type);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /** Used to get the current working dir when displaying the animation. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    LibraryWindow& libraryWindow;

    /** Used to know when the user selects a different element. */
    AnimationElementsWindow& animationElementsWindow;

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

    /** The gizmo for editing the animation's entity alignment anchor. */
    PointGizmo entityAlignmentAnchorGizmo;

    /** Holds the AnimationTimeline so it can be scrolled horizontally. */
    AUI::ScrollArea timelineScrollArea;

    /** The timeline for editing the sprites in the animation. */
    AnimationTimeline* timeline;

    AUI::Text descText;
};

} // End namespace ResourceImporter
} // End namespace AM
