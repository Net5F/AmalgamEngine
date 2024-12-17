#pragma once

#include "TilePosition.h"
#include "Vector3.h"
#include "BoundingBox.h"
#include "Ray.h"
#include "SharedConfig.h"
#include <SDL_rect.h>
#include <optional>

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
     * Note: Since this is just a simple conversion, we don't do any bounds 
     *       checking. It's on you to make sure the returned value is valid.
     */
    static Vector3 screenToWorldMinimum(const SDL_FPoint& screenPoint,
                                        const Camera& camera);

    /**
     * Converts a point in screen space to a point in world space, with 
     * Z == camera.target.z.
     * @return If screenPoint doesn't intersect the camera's view bounds, 
     *         returns null.
     */
    static std::optional<Vector3>
        screenToWorldTarget(const SDL_FPoint& screenPoint,
                            const Camera& camera);

    /**
     * Converts a point in screen space to a ray in world space.
     * @return If successful, returns a ray starting at the closest intersection
     *         between screenPoint and the camera's view bounds, pointing in 
     *         the normalized direction that the camera is facing.
     *         If screenPoint doesn't intersect the camera's view bounds, 
     *         returns null.
     *
     * Note: This can return null if the camera is zoomed out. We don't change
     *       the camera position or expand the view bounds when zooming (zoom 
     *       is just done by scaling), so you can end up seeing outside of the 
     *       camera's view bounds.
     */
    static std::optional<Ray> screenToWorldRay(const SDL_FPoint& screenPoint,
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
     * Centers the given model-space bounding volume on the given position.
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
