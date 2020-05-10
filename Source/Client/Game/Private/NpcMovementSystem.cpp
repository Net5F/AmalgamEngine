#include "NpcMovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

NpcMovementSystem::NpcMovementSystem(Game& inGame, World& inWorld,
                                             Network& inNetwork)
: game(inGame), world(inWorld), network(inNetwork)
{
}

void NpcMovementSystem::processMovements(float deltaSeconds)
{
    for (unsigned int entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        /* Move all NPCs that have an input, position, and movement component. */
        if (entityID != world.playerID
        && (world.componentFlags[entityID] & ComponentFlag::Input)
        && (world.componentFlags[entityID] & ComponentFlag::Position)
        && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
            // Save the old positions.
            world.oldPositions[entityID].x = world.positions[entityID].x;
            world.oldPositions[entityID].y = world.positions[entityID].y;

            // Process their movement.
            MovementHelpers::moveEntity(world.positions[entityID],
                world.movements[entityID], world.inputs[entityID].inputStates,
                deltaSeconds);
        }
    }
}

} // namespace Client
} // namespace AM
