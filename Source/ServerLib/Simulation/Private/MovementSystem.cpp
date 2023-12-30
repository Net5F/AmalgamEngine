#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Rotation.h"
#include "Collision.h"
#include "SharedConfig.h"
#include "Transforms.h"
#include "Log.h"
#include "tracy/Tracy.hpp"

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
    auto group
        = world.registry
              .group<Input, Position, PreviousPosition, Rotation, Collision>();
    for (auto [entity, input, position, previousPosition, rotation, collision] :
         group.each()) {
        // Save their old position.
        previousPosition = position;

        // Calculate their desired next position.
        Position desiredPosition{position};
        desiredPosition = MovementHelpers::calcPosition(
            position, input.inputStates, SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update the direction they're facing, based on their current inputs.
        rotation = MovementHelpers::calcRotation(rotation, input.inputStates);

        // If they're trying to move, resolve collisions.
        if (desiredPosition != position) {
            // Calculate a new bounding box to match their desired position.
            BoundingBox desiredBounds{Transforms::modelToWorldCentered(
                collision.modelBounds, desiredPosition)};

            // Resolve any collisions with the surrounding bounding boxes.
            BoundingBox resolvedBounds{MovementHelpers::resolveCollisions(
                collision.worldBounds, desiredBounds, entity, world.registry,
                world.tileMap, world.entityLocator)};

            // Update their bounding box and position.
            // Note: Since desiredBounds was properly offset, we can do a
            //       simple diff to get the position.
            position += (resolvedBounds.getMinPosition()
                         - collision.worldBounds.getMinPosition());
            collision.worldBounds = resolvedBounds;
        }

        // If they did actually move, update their position in the locator.
        if (position != previousPosition) {
            world.entityLocator.setEntityLocation(entity,
                                                  collision.worldBounds);
        }
    }
}

} // namespace Server
} // namespace AM
