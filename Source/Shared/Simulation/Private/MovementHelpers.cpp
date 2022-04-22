#include "MovementHelpers.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Boundingbox.h"
#include "Ignore.h"

namespace AM
{
Velocity MovementHelpers::updateVelocity(const Velocity& velocity,
                                         const Input::StateArr& inputStates,
                                         double deltaSeconds)
{
    // TODO: Ignoring while velocity is constant for testing.
    ignore(deltaSeconds);

    Velocity updatedVelocity{velocity};

    static constexpr double VELOCITY{30};
    // Y-axis (favors up).
    if (inputStates[Input::YUp] == Input::Pressed) {
        updatedVelocity.y = -VELOCITY;
    }
    else if (inputStates[Input::YDown] == Input::Pressed) {
        updatedVelocity.y = VELOCITY;
    }
    else {
        updatedVelocity.y = 0;
    }

    // X-axis (favors up).
    if (inputStates[Input::XUp] == Input::Pressed) {
        updatedVelocity.x = VELOCITY;
    }
    else if (inputStates[Input::XDown] == Input::Pressed) {
        updatedVelocity.x = -VELOCITY;
    }
    else {
        updatedVelocity.x = 0;
    }

    // Z-axis (favors up).
    if (inputStates[Input::ZUp] == Input::Pressed) {
        updatedVelocity.z = VELOCITY;
    }
    else if (inputStates[Input::ZDown] == Input::Pressed) {
        updatedVelocity.z = -VELOCITY;
    }
    else {
        updatedVelocity.z = 0;
    }

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
    newPosition.z += static_cast<float>((deltaSeconds * velocity.z));

    return newPosition;
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
