#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "MovementModifiers.h"
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
, entityMover{world.registry, world.tileMap, world.entityLocator,
              world.collisionLocator}
{
}

void MovementSystem::processMovements()
{
    ZoneScoped;

    // Move all entities that have the required components.
    auto group
        = world.registry.group<Input, Position, PreviousPosition, Movement,
                               MovementModifiers, Rotation, Collision>();
    for (auto [entity, input, position, previousPosition, movement,
               movementMods, rotation, collision] : group.each()) {
        // Save their old position.
        previousPosition = position;

        // Move the entity.
        entityMover.moveEntity(entity, input.inputStates, position,
                               previousPosition, movement, movementMods,
                               rotation, collision,
                               SharedConfig::SIM_TICK_TIMESTEP_S);
    }
}

} // namespace Server
} // namespace AM
