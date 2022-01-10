#pragma once

#include "TilePosition.h"
#include <SDL2/SDL_rect.h>

namespace AM
{
class Position;
class ScreenPoint;
class Sprite;
class Camera;
class BoundingBox;

/**
 * Static functions for transforming between world and screen space.
 */
class Transforms
{
public:
    /**
     * Converts a position in world space to a point in screen space.
     *
     * @param zoomFactor  The camera's zoom factor.
     */
    static ScreenPoint worldToScreen(const Position& position,
                                     float zoomFactor);

    /**
     * Converts a Z coordinate in world space to a Y coordinate in screen space.
     *
     * @param zoomFactor  The camera's zoom factor.
     */
    static float worldZToScreenY(float zCoord, float zoomFactor);

    /**
     * Converts a point in screen space to a position in world space.
     */
    static Position screenToWorld(const ScreenPoint& screenPoint,
                                  const Camera& camera);

    /**
     * Converts a Y coordinate in screen space to a Z coordinate in world space.
     *
     * @param zoomFactor  The camera's zoom factor.
     */
    static float screenYToWorldZ(float yCoord, float zoomFactor);

    /**
     * Helper function, converts a camera-relative screen point to a tile
     * position.
     *
     * Mostly used for getting the tile that the mouse is over.
     */
    static TilePosition screenToTile(const ScreenPoint& screenPoint,
                                     const Camera& camera);

    /**
     * Converts a model-space bounding box to a world-space box, placed at the
     * given position.
     *
     * Mostly used to place a bounding box associated with a tile.
     */
    static BoundingBox modelToWorld(const BoundingBox& modelBounds,
                                    const Position& position);

    /**
     * Converts a model-space bounding box to a world-space box, centered on
     * the given position.
     *
     * Mostly used to center a bounding box on an entity's position.
     */
    static BoundingBox modelToWorldCentered(const BoundingBox& modelBounds,
                                            const Position& position);
};

} // End namespace AM
