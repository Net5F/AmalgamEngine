#pragma once

#include "TilePosition.h"
#include "Position.h"
#include "BoundingBox.h"
#include "Ray.h"
#include "SharedConfig.h"
#include <SDL_rect.h>

namespace AM
{
struct Sprite;
struct Camera;

/**
 * Static functions for transforming between world, screen, and model space.
 */
class Transforms
{
public:
    /**
     * Converts a position in world space to a point in screen space.
     *
     * @param zoomFactor  The camera's zoom factor.
     */
    static SDL_FPoint worldToScreen(const Position& position, float zoomFactor);

    /**
     * Converts a Z coordinate in world space to a Y coordinate in screen space.
     *
     * @param zoomFactor  The camera's zoom factor.
     */
    static float worldZToScreenY(float zCoord, float zoomFactor);

    /**
     * Converts a point in screen space to a position in world space, with 
     * Z == 0.
     */
    static Position screenToWorldMinimum(const SDL_FPoint& screenPoint,
                                         const Camera& camera);

    /**
     * Converts a point in screen space to a position in world space, with 
     * Z == camera.target.z.
     */
    static Position screenToWorldTarget(const SDL_FPoint& screenPoint,
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
     * Converts a model-space bounding volume to a world-space volume, starting  
     * at the given position.
     *
     * Mostly used for tiles, since their bounds start at their coordinate.
     */
    static BoundingBox modelToWorld(const BoundingBox& modelBounds,
                                    const Position& position);

    /**
     * Converts a model-space bounding volume to a world-space volume, centered 
     * on the given position.
     *
     * Mostly used for entities, since their bounds are centered on their 
     * position.
     *
     * Note: This function takes care to center the bounds based on the
     *       sprite's "stage size". If you naively center based on the size of
     *       the box, you won't get the correct positioning.
     */
    static BoundingBox modelToWorldCentered(const BoundingBox& modelBounds,
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
