#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "EnttGroups.h"
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
    auto movementGroup = EnttGroups::getMovementGroup(world.registry);
    for (auto [entity, input, position, previousPosition, movement,
               movementMods, rotation, collision] : movementGroup.each()) {
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
