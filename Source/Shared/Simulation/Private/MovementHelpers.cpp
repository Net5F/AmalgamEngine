#include "MovementHelpers.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Boundingbox.h"
#include "Ignore.h"

namespace AM
{
void MovementHelpers::updateVelocity(Velocity& velocity,
                                     Input::StateArr& inputStates,
                                     double deltaSeconds)
{
    // TODO: Ignoring while velocity is constant for testing.
    ignore(deltaSeconds);

    static constexpr double VELOCITY{30};
    // Y-axis (favors up).
    if (inputStates[Input::YUp] == Input::Pressed) {
        velocity.y = -VELOCITY;
    }
    else if (inputStates[Input::YDown] == Input::Pressed) {
        velocity.y = VELOCITY;
    }
    else {
        velocity.y = 0;
    }

    // X-axis (favors up).
    if (inputStates[Input::XUp] == Input::Pressed) {
        velocity.x = VELOCITY;
    }
    else if (inputStates[Input::XDown] == Input::Pressed) {
        velocity.x = -VELOCITY;
    }
    else {
        velocity.x = 0;
    }

    // Z-axis (favors up).
    if (inputStates[Input::ZUp] == Input::Pressed) {
        velocity.z = VELOCITY;
    }
    else if (inputStates[Input::ZDown] == Input::Pressed) {
        velocity.z = -VELOCITY;
    }
    else {
        velocity.z = 0;
    }
}

void MovementHelpers::updatePosition(Position& position, Velocity& velocity,
                                     double deltaSeconds)
{
    // Update the position.
    position.x += (deltaSeconds * velocity.x);
    position.y += (deltaSeconds * velocity.y);
    position.z += (deltaSeconds * velocity.z);
}

Position MovementHelpers::interpolatePosition(PreviousPosition& previousPos,
                                              Position& position, double alpha)
{
    double interpX{(position.x * alpha) + (previousPos.x * (1.0 - alpha))};
    double interpY{(position.y * alpha) + (previousPos.y * (1.0 - alpha))};
    double interpZ{(position.z * alpha) + (previousPos.z * (1.0 - alpha))};
    return {static_cast<float>(interpX), static_cast<float>(interpY),
            static_cast<float>(interpZ)};
}

} // End namespace AM
