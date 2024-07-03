#pragma once

#include "Input.h"
#include "Vector3.h"
#include "BoundingBox.h"
#include "Tile.h"
#include "TileExtent.h"
#include "Rotation.h"
#include "Log.h"
#include "entt/fwd.hpp"
#include <array>

namespace AM
{
struct Position;
struct PreviousPosition;
struct Movement;
struct Collision;
class TileMapBase;
class EntityLocator;

/**
 * Shared static functions for moving entities.
 */
class MovementHelpers
{
public:
    /**
     * Calculates an updated velocity based on the given data.

     * @param inputStates The current input state.
     * @param[out] movement The current movement state (may update velocity 
     *                      and jumpCount).
     * @param deltaSeconds The number of seconds that have passed since the
     *                     last update.
     *
     * @return The updated velocity.
     */
    static Vector3 calcVelocity(const Input::StateArr& inputStates,
                                Movement& movement, double deltaSeconds);

    /**
     * Calculates an updated position based on the given data.
     *
     * @param position The current position.
     * @param velocity The current velocity.
     * @param deltaSeconds The number of seconds that have passed since the
     *                     last update.
     *
     * @return The updated position.
     */
    static Position calcPosition(const Position& position,
                                 const Vector3& velocity, double deltaSeconds);

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

private:
    /**
     * Returns the appropriate direction for the given direction int.
     * @param directionInt An integer representation of a direction, derived
     *                     from the formula (3 * (yDown - yUp) + xUp - xDown).
     */
    static Rotation::Direction directionIntToDirection(int directionInt);
};

} // End namespace AM
