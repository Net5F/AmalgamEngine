#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "BoundingBox.h"
#include "PositionHasChanged.h"
#include "SharedConfig.h"
#include "Transforms.h"
#include "Log.h"
#include "Profiler.h"

namespace AM
{
namespace Server
{
MovementSystem::MovementSystem(World& inWorld)
: world(inWorld)
{
}

void MovementSystem::processMovements()
{
    SCOPED_CPU_SAMPLE(processMovements);

    // Move all entities that have the required components.
    auto group = world.registry.group<Input, Position, PreviousPosition,
                                      Velocity, BoundingBox>();
    for (entt::entity entity : group) {
        auto [input, position, previousPosition, velocity, boundingBox]
            = group.get<Input, Position, PreviousPosition, Velocity,
                        BoundingBox>(entity);

        // Save their old position.
        previousPosition = position;

        // Use the current input state to update their velocity for this tick.
        velocity = MovementHelpers::updateVelocity(velocity, input.inputStates,
                                        SharedConfig::SIM_TICK_TIMESTEP_S);

        // Calculate their desired position, using the new velocity.
        Position desiredPosition{position};
        desiredPosition = MovementHelpers::updatePosition(desiredPosition, velocity,
                                        SharedConfig::SIM_TICK_TIMESTEP_S);

        // If they're trying to move, resolve the movement.
        if (desiredPosition != position) {
            // Calculate a new bounding box to match their desired position.
            BoundingBox desiredBounds{MovementHelpers::moveBoundingBox(boundingBox, position,
                desiredPosition)};

            // Resolve any collisions with the surrounding bounding boxes.
            BoundingBox resolvedBounds{MovementHelpers::resolveCollisions(boundingBox,
                desiredBounds, world.tileMap)};

            // Update their bounding box and position.
            boundingBox = resolvedBounds;
            position = resolvedBounds.asEntityPosition();
        }

        // If they did actually move.
        if (position != previousPosition) {
            // Update their position in the locator.
            world.entityLocator.setEntityLocation(entity, boundingBox);

            // Tag them as having moved.
            world.registry.emplace<PositionHasChanged>(entity);
        }
    }
}

} // namespace Server
} // namespace AM
