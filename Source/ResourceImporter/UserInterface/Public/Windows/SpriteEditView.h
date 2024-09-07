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

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render() override;

private:
    /** The transparency value for the stage graphic. */
    static constexpr float STAGE_ALPHA{127};

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

    /**
     * Transforms the vertices that make up the stage's bottom face from world
     * space to screen space, scales them to the current UI scaling, and 
     * offsets them using the current offsets.
     *
     * The finished points are pushed into the given vector in the order:
     *     (minX, minY, minZ), (maxX, minY, minZ), (maxX, maxY, minZ),
     *     (minX, maxY, minZ)
     */
    void calcStageScreenPoints(std::vector<SDL_Point>& stageScreenPoints);

    /**
     * Moves the stage graphic coords to their proper screen position.
     */
    void moveStageGraphic(std::vector<SDL_Point>& stageScreenPoints);

    /**
     * Renders the stage's bottom face.
     */
    void renderStage(const SDL_Point& windowTopLeft);

    /** Used to get the current working dir when displaying the sprite. */
    DataModel& dataModel;

    /** The active sprite's ID. */
    SpriteID activeSpriteID;

    // Stage (1 polygon, 4 coordinates)
    std::array<Sint16, 4> stageXCoords;
    std::array<Sint16, 4> stageYCoords;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;

    /** Checkerboard image, tiled as the background for the loaded sprite. */
    AUI::Image checkerboardImage;

    /** The sprite that is currently loaded onto the stage. */
    AUI::Image spriteImage;

    /** The gizmo for editing the sprite's bounding box. */
    BoundingBoxGizmo boundingBoxGizmo;

    AUI::Text descText;
};

} // End namespace ResourceImporter
} // End namespace AM
