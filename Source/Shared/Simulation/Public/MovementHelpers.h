#pragma once

#include "Input.h"
#include "BoundingBox.h"
#include "TileExtent.h"
#include "EmptySpriteID.h"
#include "Log.h"
#include <array>

namespace AM
{
struct Position;
struct PreviousPosition;
struct Velocity;

/**
 * Shared static functions for moving entities.
 */
class MovementHelpers
{
public:
    /**
     * Constant acceleration.
     * TODO: Eventually move this to be dynamic based on the player stats.
     */
    static constexpr float acceleration = 750;

    /**
     * Updates the given velocity based on the given inputStates.
     *
     * @param velocity  The velocity to update.
     * @param inputStates  The current input state.
     * @param deltaSeconds  The number of seconds that have passed since the
     *                      last update.
     *
     * @return The updated velocity.
     */
    static Velocity updateVelocity(const Velocity& velocity,
                                   const Input::StateArr& inputStates,
                                   double deltaSeconds);

    /**
     * Updates the given position based on the given velocity.
     *
     * @param[out] position  The position to update.
     * @param velocity  The current velocity.
     * @param deltaSeconds  The number of seconds that have passed since the
     *                      last update.
     *
     * @return The updated position.
     */
    static Position updatePosition(const Position& position,
                                   const Velocity& velocity,
                                   double deltaSeconds);

    /**
     * Returns a position interpolated between previousPos and position.
     */
    static Position interpolatePosition(const PreviousPosition& previousPos,
                                        const Position& position, double alpha);

    /**
     * Resolves collisions between the given desiredBox and other nearby
     * bounding boxes in the world.
     *
     * @param currentBounds  The bounding box, at its current position.
     * @param desiredBounds  The bounding box, at its desired position.
     * @param tileMap  The world's tile map.
     *
     * @return The desired bounding box, moved to resolve collisions.
     */
    template<typename T>
    static BoundingBox resolveCollisions(const BoundingBox& currentBounds,
                                         const BoundingBox& desiredBounds,
                                         const T& tileMap)
    {
        // TODO: Replace this logic with real sliding collision.

        // If the desired movement would go outside of the map, don't let
        // them move.
        TileExtent boxTileExtent{desiredBounds.asTileExtent()};
        TileExtent mapExtent{tileMap.getTileExtent()};
        if (!mapExtent.containsExtent(boxTileExtent)) {
            return currentBounds;
        }

        // For each tile that the desired bounds is touching.
        int yMax{boxTileExtent.y + boxTileExtent.yLength};
        int xMax{boxTileExtent.x + boxTileExtent.xLength};
        for (int y = boxTileExtent.y; y < yMax; ++y) {
            for (int x = boxTileExtent.x; x < xMax; ++x) {
                const auto& tile{tileMap.getTile(x, y)};

                // For each sprite layer in this tile.
                for (const auto& layer : tile.spriteLayers) {
                    // If this layer doesn't have a bounding box, skip it.
                    if ((layer.sprite->numericID == EMPTY_SPRITE_ID)
                        || !(layer.sprite->hasBoundingBox)) {
                        continue;
                    }

                    // If the desired movement would intersect a box, don't let
                    // them move.
                    if (desiredBounds.intersects(layer.worldBounds)) {
                        return currentBounds;
                    }
                }
            }
        }

        return desiredBounds;
    }
};

} // End namespace AM
