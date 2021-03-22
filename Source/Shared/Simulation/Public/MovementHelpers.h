#pragma once

#include "Input.h"
#include <array>

namespace AM
{
class Position;
class PreviousPosition;
class Movement;
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
     * Moves the given PositionComponent and MovementComponent based on the
     * given inputStates and deltaSeconds.
     *
     * @post The given position and movement components are modified in-place to
     * the new data.
     */
    static void moveEntity(Position& position, Movement& movement,
                           Input::StateArr& inputStates, double deltaSeconds);

    /**
     * Returns a position interpolated between previousPos and position.
     */
    static Position interpolatePosition(PreviousPosition& previousPos,
                                        Position& position, double alpha);

    /**
     * Moves a sprite's world bounds to the given position.
     */
    static void moveSpriteWorldBounds(Position& position, Sprite& sprite);

private:
    /**
     * Moves the given MovementComponent based on the given inputStates and
     * deltaSeconds.
     *
     * @post The given movement component is modified in-place to the new data.
     */
    static void updateVelocity(Movement& movement, Input::StateArr& inputStates,
                               double deltaSeconds);
};

} // namespace AM
