#include "MovementSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "MovementHelpers.h"
#include "EnttGroups.h"
#include "SharedConfig.h"
#include "Transforms.h"
#include "Log.h"
#include "tracy/Tracy.hpp"

namespace AM
{
namespace Server
{
MovementSystem::MovementSystem(const SimulationContext& inSimContext)
: world(inSimContext.simulation.getWorld())
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
               movementMods, rotation, collision, collisionBitSets] :
         movementGroup.each()) {
        // Save their old position.
        previousPosition = position;

        // Move the entity.
        entityMover.moveEntity(
            {.entity{entity},
             .inputStates{input.inputStates},
             .position{position},
             .previousPosition{previousPosition},
             .movement{movement},
             .movementMods{movementMods},
             .rotation{rotation},
             .collision{collision},
             .collisionBitSets{collisionBitSets},
             .deltaSeconds{SharedConfig::SIM_TICK_TIMESTEP_S}});
    }
}

} // namespace Server
} // namespace AM
