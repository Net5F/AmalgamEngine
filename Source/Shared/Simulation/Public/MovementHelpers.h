#pragma once

#include "Input.h"
#include <array>

namespace AM
{
class Position;
class PreviousPosition;
class Velocity;
class Sprite;

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
     * Uses the given input state and time delta to update the given velocity.
     *
     * @post The given velocity component is modified in-place to the new data.
     */
    static void updateVelocity(Velocity& velocity, Input::StateArr& inputStates,
                               double deltaSeconds);

    /**
     * Uses the given time delta and velocity to update the given position.
     *
     * @post The given position component is modified in-place to the new data.
     */
    static void updatePosition(Position& position, Velocity& velocity,
                               double deltaSeconds);

    /**
     * Returns a position interpolated between previousPos and position.
     */
    static Position interpolatePosition(PreviousPosition& previousPos,
                                        Position& position, double alpha);

private:
};

} // End namespace AM
