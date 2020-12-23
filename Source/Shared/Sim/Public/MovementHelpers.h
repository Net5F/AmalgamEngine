#pragma once

#include "Position.h"
#include "Movement.h"
#include "Input.h"
#include "GameDefs.h"
#include <array>

namespace AM
{
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
    static void moveEntity(Position& position,
                           Movement& movement,
                           Input::StateArr& inputStates, double deltaSeconds);

private:
    /**
     * Moves the given MovementComponent based on the given inputStates and
     * deltaSeconds.
     *
     * @post The given movement component is modified in-place to the new data.
     */
    static void updateVelocity(Movement& movement,
                               Input::StateArr& inputStates, double deltaSeconds);
};

} // namespace AM
