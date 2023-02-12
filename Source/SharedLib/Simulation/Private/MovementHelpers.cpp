#include "MovementHelpers.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Rotation.h"
#include "BoundingBox.h"
#include "SharedConfig.h"
#include "Ignore.h"

/** The constant to multiply by when normalizing a diagonal direction vector
    to be equal magnitude to movement in cardinal directions.
    sin(45) == cos(45) == 0.70710678118 */
const float DIAGONAL_NORMALIZATION_CONSTANT{0.70710678118f};

namespace AM
{
Velocity MovementHelpers::updateVelocity(const Velocity& velocity,
                                         const Input::StateArr& inputStates,
                                         double deltaSeconds)
{
    // Note: Ignoring while velocity is constant for testing.
    //       If we ever care to support non-constant velocity, we'll need this.
    ignore(deltaSeconds);

    // Direction values. 0 == no movement, 1 == movement.
    int xUp{static_cast<int>(inputStates[Input::XUp])};
    int xDown{static_cast<int>(inputStates[Input::XDown])};
    int yUp{static_cast<int>(inputStates[Input::YUp])};
    int yDown{static_cast<int>(inputStates[Input::YDown])};

    // Calculate our direction vector, based on the entity's inputs.
    // Note: Opposite inputs cancel eachother out.
    float xDirection{static_cast<float>(xUp - xDown)};
    float yDirection{static_cast<float>(yUp - yDown)};

    // If we're moving diagonally, normalize our direction vector.
    if ((xDirection != 0) && (yDirection != 0)) {
        xDirection *= DIAGONAL_NORMALIZATION_CONSTANT;
        yDirection *= DIAGONAL_NORMALIZATION_CONSTANT;
    }

    // Apply the velocity.
    Velocity updatedVelocity{velocity};
    updatedVelocity.x = xDirection * SharedConfig::MOVEMENT_VELOCITY;
    updatedVelocity.y = yDirection * SharedConfig::MOVEMENT_VELOCITY;

    // Note: Disabled Z-axis since it's underdeveloped. Can re-enable for
    //       for testing. Eventually, we'll incorporate it fully.
    // int zUp{static_cast<int>(inputStates[Input::ZUp])};
    // int zDown{static_cast<int>(inputStates[Input::ZDown])};
    // float zDirection{static_cast<float>(zUp - zDown)};
    // updatedVelocity.z = zDirection * SharedConfig::MOVEMENT_VELOCITY;

    return updatedVelocity;
}

Position MovementHelpers::updatePosition(const Position& position,
                                         const Velocity& velocity,
                                         double deltaSeconds)
{
    // Update the position.
    Position newPosition{position};
    newPosition.x += static_cast<float>((deltaSeconds * velocity.x));
    newPosition.y += static_cast<float>((deltaSeconds * velocity.y));

    // Note: Disabled Z-axis since it's underdeveloped. Can re-enable for
    //       for testing. Eventually, we'll incorporate it fully.
    // newPosition.z += static_cast<float>((deltaSeconds * velocity.z));

    return newPosition;
}

Rotation MovementHelpers::updateRotation(const Rotation& rotation,
                                         const Input::StateArr& inputStates)
{
    // Direction values. 0 == no movement, 1 == movement.
    int xUp{static_cast<int>(inputStates[Input::XUp])};
    int xDown{static_cast<int>(inputStates[Input::XDown])};
    int yUp{static_cast<int>(inputStates[Input::YUp])};
    int yDown{static_cast<int>(inputStates[Input::YDown])};

    // Calculate which direction the entity is facing, based on its inputs.
    // Note: Opposite inputs cancel eachother out.
    int directionInt{3 * (yDown - yUp) + xUp - xDown};
    Rotation::Direction direction{
        static_cast<Rotation::Direction>(directionInt)};

    switch (direction) {
        case Rotation::Direction::None: {
            // No inputs or canceling inputs, keep the current direction.
            return rotation;
        }
        default: {
            return {direction};
        }
    }
}

Position
    MovementHelpers::interpolatePosition(const PreviousPosition& previousPos,
                                         const Position& position, double alpha)
{
    double interpX{(position.x * alpha) + (previousPos.x * (1.0 - alpha))};
    double interpY{(position.y * alpha) + (previousPos.y * (1.0 - alpha))};
    double interpZ{(position.z * alpha) + (previousPos.z * (1.0 - alpha))};
    return {static_cast<float>(interpX), static_cast<float>(interpY),
            static_cast<float>(interpZ)};
}

} // End namespace AM
