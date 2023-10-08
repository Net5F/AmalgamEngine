#pragma once

#include "Input.h"
#include "BoundingBox.h"
#include "Tile.h"
#include "TileExtent.h"
#include "EmptySpriteID.h"
#include "Log.h"
#include "entt/fwd.hpp"
#include <array>

namespace AM
{
struct Position;
struct PreviousPosition;
struct Rotation;
class TileMapBase;
class EntityLocator;

/**
 * Shared static functions for moving entities.
 */
class MovementHelpers
{
public:
    /**
     * Calculates an updated position based on the current position, input state, 
     * and a time delta.
     *
     * @param position The current position.
     * @param inputStates The current input state.
     * @param deltaSeconds The number of seconds that have passed since the
     *                     last update.
     *
     * @return The updated position.
     */
    static Position calcPosition(const Position& position,
                                 const Input::StateArr& inputStates,
                                 double deltaSeconds);

    /**
     * Calculates a rotation based on the given input state.
     * If there are no inputs or they cancel out, the given rotation will be 
     * returned.
     *
     * @param rotation The current rotation.
     * @param inputStates The current input state.
     */
    static Rotation calcRotation(const Rotation& rotation,
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
     * @param currentBounds The bounding box, at its current position.
     * @param desiredBounds The bounding box, at its desired position.
     * @param movingEntity The entity that's trying to move.
     * @param registry The world's entity registry.
     * @param tileMap The world's tile map.
     * @param entityLocator The world's entity locator.
     *
     * @return The desired bounding box, moved to resolve collisions.
     */
    static BoundingBox resolveCollisions(const BoundingBox& currentBounds,
                                         const BoundingBox& desiredBounds,
                                         entt::entity movingEntity,
                                         const entt::registry& registry,
                                         const TileMapBase& tileMap,
                                         EntityLocator& entityLocator);

private:
    /**
     * Returns the appropriate direction for the given direction int.
     * @param directionInt An integer representation of a direction, derived 
     *                     from the formula (3 * (yDown - yUp) + xUp - xDown).
     */
    static Rotation::Direction directionIntToDirection(int directionInt);
};

} // End namespace AM
