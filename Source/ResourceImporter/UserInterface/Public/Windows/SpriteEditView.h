#pragma once

#include "LibraryItemData.h"
#include "StageGraphic.h"
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
 * The center stage shown when the user loads a sprite from the Library.
 * Allows the user to edit the active sprite's bounding box.
 */
class SpriteEditView : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteEditView(DataModel& inDataModel);

private:
    /**
     * If the new active item is a sprite, loads it's data onto this stage.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * If the changed sprite is currently active, updates this stage to reflect 
     * the new data.
     */
    void onSpriteModelBoundsIDChanged(SpriteID spriteID,
                                      BoundingBoxID newModelBoundsID);
    void onSpriteCustomModelBoundsChanged(
        SpriteID spriteID, const BoundingBox& newCustomModelBounds);
    void onSpriteStageOriginChanged(SpriteID spriteID,
                                    const SDL_Point& newStageOrigin);

    /**
     * (If active sprite was removed) Sets activeSprite to invalid and returns
     * the stage to its default state.
     */
    void onSpriteRemoved(SpriteID spriteID);

    /**
     * Pushes the gizmo's updated bounding box to the model.
     */
    void onGizmoBoundingBoxUpdated(const BoundingBox& updatedBounds);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /** Used to get the current working dir when displaying the sprite. */
    DataModel& dataModel;

    /** The active sprite's ID. */
    SpriteID activeSpriteID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;

    /** Checkerboard image, tiled as the background for the loaded sprite. */
    AUI::Image checkerboardImage;

    /** The transparent graphic that shows the stage bounds. */
    StageGraphic stageGraphic;

    /** The sprite that is currently loaded onto the stage. */
    AUI::Image spriteImage;

    /** The gizmo for editing the sprite's bounding box. */
    BoundingBoxGizmo boundingBoxGizmo;

    AUI::Text descText;
};

} // End namespace ResourceImporter
} // End namespace AM
