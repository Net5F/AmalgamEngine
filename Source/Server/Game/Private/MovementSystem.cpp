#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Log.h"

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
    for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        /* Move all entities that have an input, position, and movement
         * component. */
        if ((world.componentFlags[entityID] & ComponentFlag::Input)
            && (world.componentFlags[entityID] & ComponentFlag::Position)
            && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
            // Process their movement.
            MovementHelpers::moveEntity(
                world.positions[entityID], world.movements[entityID],
                world.inputs[entityID].inputStates, GAME_TICK_TIMESTEP_S);
        }
    }
}

} // namespace Server
} // namespace AM
