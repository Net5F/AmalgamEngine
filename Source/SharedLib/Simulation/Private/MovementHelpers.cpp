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
    // Update the entity's velocity, depending on whether it can fly or not.
    Vector3 updatedVelocity{};
    if (movementMods.canFly) {
        updatedVelocity
            = calcVelocityCanFly(inputStates, movement, movementMods);
    }
    else {
        updatedVelocity
            = calcVelocityNoFly(inputStates, movement, movementMods);
    }

    // Apply the project's velocity mod.
    updatedVelocity += movementMods.velocityMod;

    // Clamp Z to the terminal velocity.
    updatedVelocity.z
        = std::max(updatedVelocity.z, SharedConfig::TERMINAL_VELOCITY);

    // If the entity had Z velocity prior to this update, assume they're 
    // falling. When they collide with the ground, this will be set to false 
    // by the collision logic.
    if (movement.velocity.z != 0) {
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

Vector3
    MovementHelpers::calcVelocityCanFly(const Input::StateArr& inputStates,
                                        Movement& movement,
                                        const MovementModifiers& movementMods)
{
    Vector3 updatedVelocity{movement.velocity};

    // Direction values. 0 == no movement, 1 == movement.
    int xUp{static_cast<int>(inputStates[Input::XUp])};
    int xDown{static_cast<int>(inputStates[Input::XDown])};
    int yUp{static_cast<int>(inputStates[Input::YUp])};
    int yDown{static_cast<int>(inputStates[Input::YDown])};
    int zUp{static_cast<int>(inputStates[Input::Jump])};
    int zDown{static_cast<int>(inputStates[Input::Crouch])};

    // Calculate their direction vector, based on their inputs.
    // Note: Opposite inputs cancel eachother out.
    float xDirection{static_cast<float>(xUp - xDown)};
    float yDirection{static_cast<float>(yUp - yDown)};
    float zDirection{static_cast<float>(zUp - zDown)};

    // If they're moving diagonally, normalize their direction vector.
    if ((xDirection != 0) && (yDirection != 0)) {
        xDirection *= DIAGONAL_NORMALIZATION_CONSTANT;
        yDirection *= DIAGONAL_NORMALIZATION_CONSTANT;
    }

    // Calc the new velocities.
    updatedVelocity.x = xDirection * movementMods.runSpeed;
    updatedVelocity.y = yDirection * movementMods.runSpeed;
    updatedVelocity.z = zDirection * movementMods.runSpeed;

    // Note: Since they're flying, we don't apply gravity.

    return updatedVelocity;
}

Vector3
    MovementHelpers::calcVelocityNoFly(const Input::StateArr& inputStates,
                                       Movement& movement,
                                       const MovementModifiers& movementMods)
{
    Vector3 updatedVelocity{movement.velocity};

    /** Calc the new Z velocity. **/
    // If they're trying and able to jump, do so.
    bool didJump{false};
    if (inputStates[Input::Jump] && !(movement.jumpHeld)
        && (movement.jumpCount < movementMods.maxJumpCount)) {
        updatedVelocity.z = static_cast<float>(movementMods.jumpImpulse);
        movement.jumpCount++;
        didJump = true;
    }

    // True if the entity is jumping while already in the air.
    bool isAirJumping{movement.isFalling && didJump};
    // True if the entity is moving vertically through the air, e.g. after 
    // jumping straight up.
    bool isVerticalFalling{movement.isFalling && (updatedVelocity.x == 0)
                           && (updatedVelocity.y == 0)};

    // If they're on the ground, or just air jumped, or are falling vertically, 
    // calc their new X/Y velocity.
    // Note: The only other case is that they're falling through the air, in 
    //       which case they'll keep traveling with their current X/Y velocity.
    if (!(movement.isFalling) || isAirJumping || isVerticalFalling) {
        // Direction values. 0 == no movement, 1 == movement.
        int xUp{static_cast<int>(inputStates[Input::XUp])};
        int xDown{static_cast<int>(inputStates[Input::XDown])};
        int yUp{static_cast<int>(inputStates[Input::YUp])};
        int yDown{static_cast<int>(inputStates[Input::YDown])};

        // Calculate their direction vector, based on their inputs.
        // Note: Opposite inputs cancel eachother out.
        float xDirection{static_cast<float>(xUp - xDown)};
        float yDirection{static_cast<float>(yUp - yDown)};

        // If they're moving diagonally, normalize their direction vector.
        if ((xDirection != 0) && (yDirection != 0)) {
            xDirection *= DIAGONAL_NORMALIZATION_CONSTANT;
            yDirection *= DIAGONAL_NORMALIZATION_CONSTANT;
        }

        // Calc the new X/Y velocity.
        if (isVerticalFalling) {
            updatedVelocity.x
                = xDirection * SharedConfig::VERTICAL_FALL_MOVE_VELOCITY;
            updatedVelocity.y
                = yDirection * SharedConfig::VERTICAL_FALL_MOVE_VELOCITY;
        }
        else {
            updatedVelocity.x = xDirection * movementMods.runSpeed;
            updatedVelocity.y = yDirection * movementMods.runSpeed;
        }
    }

    // Always apply gravity.
    updatedVelocity.z -= SharedConfig::FORCE_OF_GRAVITY;

    // Update their jump held state, for use next time.
    movement.jumpHeld = inputStates[Input::Jump];

    return updatedVelocity;
}

} // End namespace AM
