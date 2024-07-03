#pragma once

#include "TilePosition.h"
#include "Vector3.h"
#include "BoundingBox.h"
#include "Ray.h"
#include "SharedConfig.h"
#include <SDL_rect.h>

namespace AM
{
struct Sprite;
struct Camera;
struct Position;

/**
 * Static functions for transforming between world, screen, and model space.
 */
class Transforms
{
public:
    /**
     * Converts a point in world space to a point in screen space.
     *
     * @param zoomFactor The camera's zoom factor.
     */
    static SDL_FPoint worldToScreen(const Vector3& point, float zoomFactor);

    /**
     * Converts a Z coordinate in world space to a Y coordinate in screen space.
     *
     * @param zoomFactor The camera's zoom factor.
     */
    static float worldZToScreenY(float zCoord, float zoomFactor);

    /**
     * Converts a point in screen space to a point in world space, with 
     * Z == 0.
     */
    static Vector3 screenToWorldMinimum(const SDL_FPoint& screenPoint,
                                        const Camera& camera);

    /**
     * Converts a point in screen space to a point in world space, with 
     * Z == camera.target.z.
     */
    static Vector3 screenToWorldTarget(const SDL_FPoint& screenPoint,
                                       const Camera& camera);

    /**
     * Converts a point in screen space to a ray in world space.
     * @return A ray starting at the closest intersection between screenPoint 
     *         and the camera's view bounds, pointing in the normalized 
     *         direction of the camera.
     */
    static Ray screenToWorldRay(const SDL_FPoint& screenPoint,
                                const Camera& camera);

    /**
     * Converts a point in screen space to a tile position in world space.
     * Note: The resulting tile will be aligned along the Z axis with the given 
     *       camera.target.z. To select higher or lower tiles, move the camera.
     */
    static TilePosition screenToWorldTile(const SDL_FPoint& screenPoint,
                                          const Camera& camera);

    /**
     * Converts a Y coordinate in screen space to a Z coordinate in world space.
     *
     * @param zoomFactor The camera's zoom factor.
     */
    static float screenYToWorldZ(float yCoord, float zoomFactor);

    /**
     * Places the given model-space bounding volume at the given tile position.
     */
    static BoundingBox modelToWorldTile(const BoundingBox& modelBounds,
                                        const TilePosition& tilePosition);

    /**
     * Places the given model-space bounding volume at the given entity position.
     *
     * Note: This function takes care to center the bounds based on the
     *       sprite's "stage size". If you naively center based on the size of
     *       the box, you won't get the correct positioning.
     */
    static BoundingBox modelToWorldEntity(const BoundingBox& modelBounds,
                                          const Position& position);

    //-------------------------------------------------------------------------
    // Constants
    //-------------------------------------------------------------------------
    /** The scaling factor to use when going from world tiles to screen tiles. */
    static constexpr float TILE_FACE_WIDTH_WORLD_TO_SCREEN{
        static_cast<float>(SharedConfig::TILE_FACE_SCREEN_WIDTH)
        / SharedConfig::TILE_WORLD_WIDTH};
    static constexpr float TILE_FACE_HEIGHT_WORLD_TO_SCREEN{
        static_cast<float>(SharedConfig::TILE_FACE_SCREEN_HEIGHT)
        / SharedConfig::TILE_WORLD_WIDTH};
    static constexpr float TILE_SIDE_HEIGHT_WORLD_TO_SCREEN{
        static_cast<float>(SharedConfig::TILE_SIDE_SCREEN_HEIGHT)
        / SharedConfig::TILE_WORLD_HEIGHT};

    /** The scaling factor to use when going from screen tiles to world tiles. */
    static constexpr float TILE_FACE_WIDTH_SCREEN_TO_WORLD{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)
        / SharedConfig::TILE_FACE_SCREEN_WIDTH};
    static constexpr float TILE_FACE_HEIGHT_SCREEN_TO_WORLD{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)
        / SharedConfig::TILE_FACE_SCREEN_HEIGHT};
    static constexpr float TILE_SIDE_HEIGHT_SCREEN_TO_WORLD{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)
        / SharedConfig::TILE_SIDE_SCREEN_HEIGHT};
};

} // End namespace AM
