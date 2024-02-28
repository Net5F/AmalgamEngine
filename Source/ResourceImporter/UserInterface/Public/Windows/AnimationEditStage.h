#pragma once

#include "LibraryItemData.h"
#include "BoundingBoxGizmo.h"
#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Text.h"
#include "AUI/Image.h"

namespace AM
{
namespace ResourceImporter
{
class DataModel;

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
    AnimationEditStage(DataModel& inDataModel);

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
    void onAnimationFpsChanged(AnimationID animationID, Uint8 newFps);
    void onAnimationFrameChanged(AnimationID animationID, Uint8 newFrameNumber,
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
    void onGizmoBoundingBoxUpdated(const BoundingBox& boundingBox);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /** Used to get the current working dir when displaying the animation. */
    DataModel& dataModel;

    /** The active animation's ID. */
    AnimationID activeAnimationID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;

    /** Checkerboard image, tiled as the background for the loaded animation. */
    AUI::Image checkerboardImage;

    /** The animation that is currently loaded onto the stage. */
    AUI::Image animationImage;

    /** The gizmo for editing the animation's bounding box. */
    BoundingBoxGizmo boundingBoxGizmo;

    AUI::Text descText1;
};

} // End namespace ResourceImporter
} // End namespace AM
