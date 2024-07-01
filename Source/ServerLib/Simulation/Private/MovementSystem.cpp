#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
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
    auto group = world.registry.group<Input, Position, PreviousPosition,
                                      Movement, Rotation, Collision>();
    for (auto [entity, input, position, previousPosition, movement, rotation,
               collision] : group.each()) {
        // If no inputs are pressed and they aren't falling, nothing needs to 
        // be done.
        if (input.inputStates.none() && !(movement.isFalling)) {
            continue;
        }

        // Save their old position.
        previousPosition = position;

        // Calculate their updated velocity.
        Velocity updatedVelocity{MovementHelpers::calcVelocity(
            input.inputStates, movement, SharedConfig::SIM_TICK_TIMESTEP_S)};
        LOG_INFO("New X velocity: %.4f", updatedVelocity.x);

        // Calculate their desired next position.
        Position desiredPosition{position};
        desiredPosition = MovementHelpers::calcPosition(
            position, updatedVelocity, SharedConfig::SIM_TICK_TIMESTEP_S);
        LOG_INFO("New X position: %.4f, %.4f", position.x, desiredPosition.x);

        // Update the direction they're facing, based on their current inputs.
        rotation = MovementHelpers::calcRotation(rotation, input.inputStates);

        // TODO: Figure out how to tell if they're falling or not, set 
        //       isFalling, and figure out how to share this with the other 
        //       locations
        // If they're trying to move, resolve collisions.
        if (desiredPosition != position) {
            // Calculate a new bounding box to match their desired position.
            BoundingBox desiredBounds{Transforms::modelToWorldCentered(
                collision.modelBounds, desiredPosition)};

            // Resolve any collisions with the surrounding bounding boxes.
            BoundingBox resolvedBounds{MovementHelpers::resolveCollisions(
                collision.worldBounds, desiredBounds, entity, world.registry,
                world.tileMap, world.entityLocator)};
            LOG_INFO("%.4f (%.4f) -> %.4f", collision.worldBounds.minX,
                     desiredBounds.minX, resolvedBounds.minX);

            // Update their bounding box and position.
            // Note: Since desiredBounds was properly offset, we can do a
            //       simple diff to get the position.
            position += (resolvedBounds.getMinPosition()
                         - collision.worldBounds.getMinPosition());
            collision.worldBounds = resolvedBounds;

            // TEMP
            // If they're moving up or down, flag them as falling.
            if (resolvedBounds.minZ != collision.worldBounds.minZ) {
                movement.isFalling = true;
            }
            else {
                movement.isFalling = false;
            }
            // TEMP
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
