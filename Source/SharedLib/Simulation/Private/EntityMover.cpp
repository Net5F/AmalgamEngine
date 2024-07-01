#include "EntityMover.h"
#include "TileMapBase.h"
#include "EntityLocator.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "Rotation.h"
#include "Collision.h"
#include "MovementHelpers.h"
#include "Transforms.h"
#include "Log.h"

namespace AM
{
EntityMover::EntityMover(const entt::registry& inRegistry,
                         const TileMapBase& inTileMap,
                         EntityLocator& inEntityLocator)
: registry{inRegistry}
, tileMap{inTileMap}
, entityLocator{inEntityLocator}
{
}

void EntityMover::moveEntity(entt::entity entity,
                             const Input::StateArr& inputStates,
                             Position& position,
                             const PreviousPosition& previousPosition,
                             Movement& movement, Rotation& rotation,
                             Collision& collision, double deltaSeconds)
{
    // If no inputs are pressed and they aren't falling, nothing needs to 
    // be done.
    if (inputStates.none() && !(movement.isFalling)) {
        return;
    }

    // Calculate their updated velocity.
    Velocity updatedVelocity{MovementHelpers::calcVelocity(
        inputStates, movement, SharedConfig::SIM_TICK_TIMESTEP_S)};

    // Calculate their desired next position.
    Position desiredPosition{position};
    desiredPosition = MovementHelpers::calcPosition(
        position, updatedVelocity, SharedConfig::SIM_TICK_TIMESTEP_S);

    // Update the direction they're facing, based on their current inputs.
    rotation = MovementHelpers::calcRotation(rotation, inputStates);

    // If they're trying to move, resolve collisions.
    if (desiredPosition != position) {
        // Calculate a new bounding box to match their desired position.
        BoundingBox desiredBounds{Transforms::modelToWorldCentered(
            collision.modelBounds, desiredPosition)};

        // Resolve any collisions with the surrounding bounding boxes.
        BoundingBox resolvedBounds{MovementHelpers::resolveCollisions(
            collision.worldBounds, desiredBounds, entity, registry,
            tileMap, entityLocator)};

        // Update their bounding box and position.
        // Note: Since desiredBounds was properly offset, we can do a
        //       simple diff to get the position.
        position += (resolvedBounds.getMinPosition()
                     - collision.worldBounds.getMinPosition());
        collision.worldBounds = resolvedBounds;
    }

    // If they did actually move, update their position in the locator.
    if (position != previousPosition) {
        entityLocator.setEntityLocation(entity, collision.worldBounds);
    }
}

} // End namespace AM
