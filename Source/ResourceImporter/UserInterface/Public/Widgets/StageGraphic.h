#pragma once

#include "AUI/Widget.h"

namespace AM
{
namespace ResourceImporter
{
/**
 * Draws the stage graphic, used to show where the bounds are when editing 
 * boxes.
 */
class StageGraphic : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    StageGraphic(const SDL_Rect& inLogicalExtent);

    /**
     * Updates the stage graphic to match the given extent and origin.
     * 
     * @param spriteTextureExtent The texture extent from the sprite data.
     * @param stageOrigin The stage origin from the sprite data.
     * @param actualSpriteImageOffset Where the sprite image will start, 
     *        relative to the top left of this widget and scaled to actual 
     *        (screen-relative) size.
     */
    void updateStage(const SDL_Rect& spriteTextureExtent,
                     const SDL_Point& stageOrigin,
                     const SDL_Point& actualSpriteImageOffset);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& windowTopLeft) override;

private:
    /** The transparency value for the stage graphic. */
    static constexpr float STAGE_ALPHA{127};

    /**
     * Transforms the vertices that make up the stage's bottom face from world
     * space to screen space, scales them to the current UI scaling, and 
     * offsets them using the current offsets.
     *
     * The finished points are pushed into the given vector in the order:
     *     (minX, minY, minZ), (maxX, minY, minZ), (maxX, maxY, minZ),
     *     (minX, maxY, minZ)
     */
    void calcStageScreenPoints(const SDL_Rect& spriteTextureExtent,
                               const SDL_Point& stageOrigin,
                               const SDL_Point& logicalSpriteImageOffset,
                               std::vector<SDL_Point>& stageScreenPoints);

    /**
     * Moves the stage graphic coords to their proper screen position.
     */
    void moveStageGraphic(std::vector<SDL_Point>& stageScreenPoints);

    /**
     * Renders the stage's bottom face.
     */
    void renderStage(const SDL_Point& windowTopLeft);

    // Stage (1 polygon, 4 coordinates)
    std::array<Sint16, 4> stageXCoords;
    std::array<Sint16, 4> stageYCoords;
};

} // End namespace ResourceImporter
} // End namespace AM
