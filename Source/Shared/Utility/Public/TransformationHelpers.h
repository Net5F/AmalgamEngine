#pragma once

#include "ScreenPoint.h"
#include "Position.h"
#include "TileIndex.h"
#include <SDL2/SDL_rect.h>

namespace AM
{
class Sprite;
class Camera;

/**
 * Static functions for transforming between world and screen space.
 */
class TransformationHelpers
{
public:
    /**
     * Converts a position in world space to a point in screen space.
     *
     * @param zoomFactor  The camera's zoom factor.
     */
    static ScreenPoint worldToScreen(const Position position, float zoomFactor);

    /**
     * Converts a Z coordinate in world space to a Y coordinate in screen space.
     *
     * @param zoomFactor  The camera's zoom factor.
     */
    static float worldZToScreenY(float zCoord, float zoomFactor);

    /**
     * Converts a point in screen space to a position in world space.
     *
     * @param zoomFactor  The camera's zoom factor.
     */
    static Position screenToWorld(const ScreenPoint screenPoint,
                                  float zoomFactor);

    /**
     * Converts a Y coordinate in screen space to a Z coordinate in world space.
     *
     * @param zoomFactor  The camera's zoom factor.
     */
    static float screenYToWorldZ(float yCoord, float zoomFactor);

    /**
     * Returns a final screen space extent for the given position, camera,
     * and sprite.
     */
    static SDL_Rect worldToScreenExtent(const Position& position,
                                        const Camera& camera,
                                        const Sprite& sprite);

    /**
     * Returns a final screen space extent for the given tile index, camera,
     * and sprite.
     *
     * Used as an alternative to worldToScreenExtent() since tile sprites have
     * some extra offsets applied.
     */
    static SDL_Rect tileToScreenExtent(const TileIndex& index,
                                       const Camera& camera,
                                       const Sprite& sprite);

    /**
     * Converts a position in world space to a tile index.
     */
    static TileIndex worldToTile(const Position& position);

    /**
     * Helper function, converts a camera-relative screen point to a tile
     * index.
     *
     * Mostly used for getting the tile that the mouse is over.
     */
    static TileIndex screenToTile(const ScreenPoint& screenPoint,
                                  const Camera& camera);
};

} // End namespace AM
