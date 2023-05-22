#pragma once

#include "Input.h"
#include "BoundingBox.h"
#include "Tile.h"
#include "TileExtent.h"
#include "EmptySpriteID.h"
#include "Log.h"
#include <array>

namespace AM
{
struct Position;
struct PreviousPosition;
struct Velocity;
struct Rotation;

/**
 * Shared static functions for moving entities.
 */
class MovementHelpers
{
public:
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
     * @param position  The position to update.
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
     * Updates the given rotation based on the given input state.
     *
     * @param rotation  The rotation to update.
     * @param inputStates  The current input state.
     *
     * @return The updated rotation.
     */
    static Rotation updateRotation(const Rotation& rotation,
                                   const Input::StateArr& inputStates);

    /**
     * Returns a position interpolated between previousPos and position.
     */
    static Position interpolatePosition(const PreviousPosition& previousPos,
                                        const Position& position, double alpha);

    // TODO: TileMap and Tile are no longer split across repos, so turn this
    //       into a normal function acting on TileMapBase.
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
        const TileExtent boxTileExtent{desiredBounds.asTileExtent()};
        const TileExtent mapExtent{tileMap.getTileExtent()};
        if (!mapExtent.containsExtent(boxTileExtent)
            || (desiredBounds.minZ < 0)) {
            return currentBounds;
        }

        // For each tile that the desired bounds is touching.
        for (int y = boxTileExtent.y; y <= boxTileExtent.yMax(); ++y) {
            for (int x = boxTileExtent.x; x <= boxTileExtent.xMax(); ++x) {
                const Tile& tile{tileMap.getTile(x, y)};

                // For each collision box in this tile.
                for (const BoundingBox& collisionBox :
                     tile.getCollisionBoxes()) {
                    // If the desired movement would intersect this box, don't 
                    // let them move.
                    if (desiredBounds.intersects(collisionBox)) {
                        return currentBounds;
                    }
                }
            }
        }

        return desiredBounds;
    }

private:
    /**
     * Returns the appropriate direction for the given direction int.
     * @param directionInt An integer representation of a direction, derived 
     *                     from the formula (3 * (yDown - yUp) + xUp - xDown).
     */
    static Rotation::Direction directionIntToDirection(int directionInt);
};

} // End namespace AM
