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
    StageGraphic(const SDL_FRect& inLogicalExtent);

    /**
     * Updates the stage graphic to match the given extent and origin.
     *
     * @param spriteTextureExtent The texture extent from the sprite data.
     * @param stageOrigin The stage origin from the sprite data.
     * @param actualSpriteImageOffset Where the sprite image will start,
     *        relative to the top left of this widget and scaled to actual
     *        (screen-relative) size.
     */
    void updateStage(const SDL_FRect& spriteTextureExtent,
                     const SDL_FPoint& stageOrigin,
                     const SDL_FPoint& actualSpriteImageOffset);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_FPoint& windowTopLeft) override;

private:
    /** The transparency value for the stage graphic. */
    static constexpr float STAGE_ALPHA{127};

    /**
     * Transforms the vertices that make up the stage's bottom face from world
     * space to screen space, scales them to the current UI scaling, and
     * offsets them using the current offsets.
     *
     * The finished points are set in the given array in the order:
     *     (minX, minY, minZ), (maxX, minY, minZ), (maxX, maxY, minZ),
     *     (minX, maxY, minZ)
     *
     * @post stageCoords is updated to the new position.
     */
    void updateStageScreenPoints(const SDL_FRect& spriteTextureExtent,
                                 const SDL_FPoint& stageOrigin,
                                 const SDL_FPoint& logicalSpriteImageOffset);

    /**
     * Moves the stage graphic coords to their proper screen position.
     */
    void moveStageGraphic(std::vector<SDL_FPoint>& stageScreenPoints);

    /**
     * Renders the stage's bottom face.
     */
    void renderStage(const SDL_FPoint& windowTopLeft);

    // Stage bottom face (1 polygon, 4 coordinates starting from top left and
    // going clockwise)
    std::array<SDL_FPoint, 4> stageCoords;
};

} // End namespace ResourceImporter
} // End namespace AM
