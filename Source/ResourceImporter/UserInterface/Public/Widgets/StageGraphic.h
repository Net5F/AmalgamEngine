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
     * The finished points are set in stageCoords in the order:
     *     (minX, minY, minZ), (maxX, minY, minZ), (maxX, maxY, minZ),
     *     (minX, maxY, minZ)
     *
     * @param spriteTextureExtent The texture extent from the sprite data.
     * @param stageOrigin The stage origin from the sprite data.
     * @param actualSpriteImageOffset Where the sprite image will start,
     *        relative to the top left of this widget and scaled to actual
     *        (screen-relative) size.
     *
     * @post stageCoords is updated to the new position and ready for rendering.
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
     * Renders the stage's bottom face.
     */
    void renderStage(const SDL_FPoint& windowTopLeft);

    // Stage bottom face (1 polygon, 4 coordinates starting from top left and
    // going clockwise)
    std::array<SDL_FPoint, 4> stageCoords;
};

} // End namespace ResourceImporter
} // End namespace AM
