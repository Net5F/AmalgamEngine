#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace Server
{
MovementSystem::MovementSystem(World& inWorld)
: world(inWorld)
{
    // Init the groups that we'll be using.
    auto group = world.registry.group<Input, Position, Movement>();
    ignore(group);
}

void MovementSystem::processMovements()
{
    /* Move all entities that have an input, position, and movement
       component. */
    auto group = world.registry.group<Input, Position, Movement>();
    for (entt::entity entity : group) {
        Input& input = group.get<Input>(entity);
        Position& position = group.get<Position>(entity);
        Movement& movement = group.get<Movement>(entity);

        // Process their movement.
        MovementHelpers::moveEntity(position, movement, input.inputStates,
            GAME_TICK_TIMESTEP_S);
    }
}

} // namespace Server
} // namespace AM
