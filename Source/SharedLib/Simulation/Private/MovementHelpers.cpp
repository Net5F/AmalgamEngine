#include "MovementHelpers.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "BoundingBox.h"
#include "SharedConfig.h"

/** The constant to multiply by when normalizing a diagonal direction vector
    to be equal magnitude to movement in cardinal directions.
    sin(45) == cos(45) == 0.70710678118 */
const float DIAGONAL_NORMALIZATION_CONSTANT{0.70710678118f};

namespace AM
{
Vector3 MovementHelpers::calcVelocity(const Input::StateArr& inputStates,
                                      Movement& movement,
                                      const MovementModifiers& movementMods)
{
    // If the entity isn't in the air (or if they can fly), calc the new X/Y 
    // velocity.
    // Note: If they're in the air, they'll keep traveling with their current 
    //       X/Y velocity.
    Vector3 updatedVelocity{movement.velocity};
    if (!(movement.isFalling) || movementMods.canFly) {
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

        // Calc the new X/Y velocity.
        updatedVelocity.x = xDirection * movementMods.runSpeed;
        updatedVelocity.y = yDirection * movementMods.runSpeed;
    }

    /** Calc the new Z velocity. **/
    // If the entity can fly and the user is trying to go up or down, treat 
    // it similar to running.
    if (movementMods.canFly) {
        if (inputStates[Input::Jump] || inputStates[Input::Crouch]) {
            int zUp{static_cast<int>(inputStates[Input::Jump])};
            int zDown{static_cast<int>(inputStates[Input::Crouch])};
            float zDirection{static_cast<float>(zUp - zDown)};

            updatedVelocity.z = zDirection * movementMods.runSpeed;
        }
        // Note: Since they're flying, we don't apply gravity.
    }
    else {
        // Not flying. If they're trying and able to jump, do so.
        if (inputStates[Input::Jump] && !(movement.jumpHeld)
            && (movement.jumpCount < movementMods.maxJumpCount)) {
            updatedVelocity.z += static_cast<float>(movementMods.jumpImpulse);
            movement.jumpCount++;
            movement.jumpHeld = true;
        }

        // Always apply gravity.
        updatedVelocity.z -= SharedConfig::FORCE_OF_GRAVITY;

        // If jump isn't held, reset our bool.
        if (!inputStates[Input::Jump]) {
            movement.jumpHeld = false;
        }
    }

    // Apply the project's velocity mod.
    updatedVelocity += movementMods.velocityMod;

    // Clamp Z to the terminal velocity.
    updatedVelocity.z
        = std::max(updatedVelocity.z, SharedConfig::TERMINAL_VELOCITY);

    if (updatedVelocity.z != 0) {
        movement.isFalling = true;
    }

    return updatedVelocity;
}

Position MovementHelpers::calcPosition(const Position& position,
                                       const Vector3& velocity,
                                       double deltaSeconds)
{
    // Update the position.
    Position newPosition{position};
    newPosition.x += static_cast<float>((deltaSeconds * velocity.x));
    newPosition.y += static_cast<float>((deltaSeconds * velocity.y));
    newPosition.z += static_cast<float>((deltaSeconds * velocity.z));

    return newPosition;
}

Rotation MovementHelpers::calcRotation(const Rotation& rotation,
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
    Rotation::Direction direction{directionIntToDirection(directionInt)};

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

Rotation::Direction MovementHelpers::directionIntToDirection(int directionInt)
{
    switch (directionInt) {
        case -4: {
            return Rotation::Direction::SouthWest;
        }
        case -3: {
            return Rotation::Direction::South;
        }
        case -2: {
            return Rotation::Direction::SouthEast;
        }
        case -1: {
            return Rotation::Direction::West;
        }
        case 0: {
            return Rotation::Direction::None;
        }
        case 1: {
            return Rotation::Direction::East;
        }
        case 2: {
            return Rotation::Direction::NorthWest;
        }
        case 3: {
            return Rotation::Direction::North;
        }
        case 4: {
            return Rotation::Direction::NorthEast;
        }
        default: {
            LOG_FATAL("Invalid direction int.");
            return Rotation::Direction::None;
        }
    }
}

} // End namespace AM
