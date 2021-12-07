#include "MovementHelpers.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "Boundingbox.h"
#include "Ignore.h"

namespace AM
{
void MovementHelpers::updateVelocity(Movement& movement,
                                     Input::StateArr& inputStates,
                                     double deltaSeconds)
{
    // TODO: Ignoring while velocity is constant for testing.
    ignore(deltaSeconds);

    static constexpr double VELOCITY{30};
    // Y-axis (favors up).
    if (inputStates[Input::YUp] == Input::Pressed) {
        movement.velY = -VELOCITY;
    }
    else if (inputStates[Input::YDown] == Input::Pressed) {
        movement.velY = VELOCITY;
    }
    else {
        movement.velY = 0;
    }

    // X-axis (favors up).
    if (inputStates[Input::XUp] == Input::Pressed) {
        movement.velX = VELOCITY;
    }
    else if (inputStates[Input::XDown] == Input::Pressed) {
        movement.velX = -VELOCITY;
    }
    else {
        movement.velX = 0;
    }

    // Z-axis (favors up).
    if (inputStates[Input::ZUp] == Input::Pressed) {
        movement.velZ = VELOCITY;
    }
    else if (inputStates[Input::ZDown] == Input::Pressed) {
        movement.velZ = -VELOCITY;
    }
    else {
        movement.velZ = 0;
    }
}

void MovementHelpers::updatePosition(Position& position, Movement& movement,
                                 double deltaSeconds)
{
    // Update the position.
    position.x += (deltaSeconds * movement.velX);
    position.y += (deltaSeconds * movement.velY);
    position.z += (deltaSeconds * movement.velZ);
}

Position MovementHelpers::interpolatePosition(PreviousPosition& previousPos,
                                              Position& position, double alpha)
{
    double interpX{(position.x * alpha) + (previousPos.x * (1.0 - alpha))};
    double interpY{(position.y * alpha) + (previousPos.y * (1.0 - alpha))};
    double interpZ{(position.z * alpha) + (previousPos.z * (1.0 - alpha))};
    return {static_cast<float>(interpX), static_cast<float>(interpY), static_cast<float>(interpZ)};
}

} // End namespace AM
