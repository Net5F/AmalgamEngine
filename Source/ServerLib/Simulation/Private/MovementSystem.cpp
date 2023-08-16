#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Rotation.h"
#include "Collision.h"
#include "SharedConfig.h"
#include "Transforms.h"
#include "Log.h"
#include "Tracy.hpp"

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
    ZoneScoped;

    // Move all entities that have the required components.
    auto group = world.registry.group<Input, Position, PreviousPosition,
                                      Velocity, Rotation, Collision>();
    for (entt::entity entity : group) {
        auto [input, position, previousPosition, velocity, rotation, collision]
            = group.get<Input, Position, PreviousPosition, Velocity, Rotation,
                        Collision>(entity);

        // Save their old position.
        previousPosition = position;

        // Update their velocity for this tick, based on their current inputs.
        velocity = MovementHelpers::updateVelocity(
            velocity, input.inputStates, SharedConfig::SIM_TICK_TIMESTEP_S);

        // Calculate their desired position, using the new velocity.
        Position desiredPosition{position};
        desiredPosition = MovementHelpers::updatePosition(
            position, velocity, SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update the direction they're facing, based on their current inputs.
        rotation = MovementHelpers::updateRotation(rotation, input.inputStates);

        // If they're trying to move, resolve collisions.
        if (desiredPosition != position) {
            // Calculate a new bounding box to match their desired position.
            BoundingBox desiredBounds{Transforms::modelToWorldCentered(
                collision.modelBounds, desiredPosition)};

            // Resolve any collisions with the surrounding bounding boxes.
            BoundingBox resolvedBounds{MovementHelpers::resolveCollisions(
                collision.worldBounds, desiredBounds, world.tileMap)};

            // Update their bounding box and position.
            // Note: Since desiredBounds was properly offset, we can do a
            //       simple diff to get the position.
            position += (resolvedBounds.getMinPosition()
                         - collision.worldBounds.getMinPosition());
            collision.worldBounds = resolvedBounds;
        }

        // If they did actually move.
        if (position != previousPosition) {
            // Update their position in the locator.
            world.entityLocator.setEntityLocation(entity,
                                                  collision.worldBounds);
        }
    }
}

} // namespace Server
} // namespace AM
