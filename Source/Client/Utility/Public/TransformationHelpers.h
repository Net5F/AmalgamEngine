#pragma once

#include "ScreenPoint.h"
#include "Position.h"
#include "TileIndex.h"
#include "SDL2pp/Rect.hh"

namespace AM
{

class Sprite;

namespace Client
{

class Camera;

/**
 * Static functions for transforming between world and screen space.
 */
class TransformationHelpers
{
public:
    /**
     * Converts a position in world space to a point in screen space.
     */
    static ScreenPoint worldToScreen(const Position position, const float zoom);

    /**
     * Converts a point in screen space to a position in world space.
     */
    static Position screenToWorld(const ScreenPoint screenPoint);

    /**
     * Returns a final screen space extent for the given position, camera,
     * and sprite.
     */
    static SDL2pp::Rect worldToScreenExtent(const Position& position, const Camera& camera, const Sprite& sprite);

    /**
     * Returns a final screen space extent for the given tile index, camera,
     * and sprite.
     *
     * Used as an alternative to worldToSpriteExtent() since tile sprites have
     * some extra offsets applied.
     */
    static SDL2pp::Rect tileToScreenExtent(const TileIndex& index, const Camera& camera, const Sprite& sprite);

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
    static TileIndex screenToTile(const ScreenPoint& screenPoint, const Camera& camera);
};

} // End namespace Client
} // End namespace AM
