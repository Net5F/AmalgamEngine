#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "BoundingBox.h"
#include "SharedConfig.h"
#include "Transforms.h"
#include "Log.h"
#include "Ignore.h"
#include "Profiler.h"

namespace AM
{
namespace Server
{
MovementSystem::MovementSystem(World& inWorld)
: world(inWorld)
{
    // Init the groups that we'll be using.
    auto group
        = world.registry.group<Input, Position, PreviousPosition, Movement, Sprite, BoundingBox>();
    ignore(group);
}

void MovementSystem::processMovements()
{
    SCOPED_CPU_SAMPLE(processMovements);

    /* Move all entities that have an input, position, and movement
       component. */
    auto group
        = world.registry.group<Input, Position, PreviousPosition, Movement, Sprite, BoundingBox>();
    for (entt::entity entity : group) {
        auto [input, position, previousPosition, movement, sprite, boundingBox]
            = group.get<Input, Position, PreviousPosition, Movement, Sprite, BoundingBox>(entity);

        // Save their old position.
        previousPosition = position;

        // Use the current input state to update their velocity for this tick.
        MovementHelpers::updateVelocity(movement, input.inputStates,
                                    SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update their position, using the new velocity.
        MovementHelpers::updatePosition(position, movement,
                                    SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update their bounding box to match the new position.
        boundingBox = Transforms::modelToWorld(sprite.modelBounds, position);

        // TODO: Tag the entity as having moved
    }
}

} // namespace Server
} // namespace AM
