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
class TileMapBase;

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
    static BoundingBox resolveCollisions(const BoundingBox& currentBounds,
                                         const BoundingBox& desiredBounds,
                                         const TileMapBase& tileMap);

private:
    /**
     * Returns the appropriate direction for the given direction int.
     * @param directionInt An integer representation of a direction, derived 
     *                     from the formula (3 * (yDown - yUp) + xUp - xDown).
     */
    static Rotation::Direction directionIntToDirection(int directionInt);
};

} // End namespace AM
