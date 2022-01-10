#include "MovementSystem.h"
#include "MovementHelpers.h"
#include "World.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "BoundingBox.h"
#include "PositionHasChanged.h"
#include "SharedConfig.h"
#include "Transforms.h"
#include "Log.h"
#include "Profiler.h"

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
    SCOPED_CPU_SAMPLE(processMovements);

    // Move all entities that have an input, position, and velocity component.
    auto group = world.registry.group<Input, Position, PreviousPosition,
                                      Velocity, BoundingBox, Sprite>();
    for (entt::entity entity : group) {
        auto [input, position, previousPosition, velocity, boundingBox, sprite]
            = group.get<Input, Position, PreviousPosition, Velocity,
                        BoundingBox, Sprite>(entity);

        // Save their old position.
        previousPosition = position;

        // Use the current input state to update their velocity for this tick.
        MovementHelpers::updateVelocity(velocity, input.inputStates,
                                        SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update their position, using the new velocity.
        MovementHelpers::updatePosition(position, velocity,
                                        SharedConfig::SIM_TICK_TIMESTEP_S);

        // If the entity moved.
        if (position != previousPosition) {
            // Update their bounding box to match their new position.
            boundingBox = Transforms::modelToWorldCentered(sprite.modelBounds,
                                                           position);

            // Update the entity's position in the locator.
            world.entityLocator.setEntityLocation(entity, boundingBox);

            // Tag the entity as having moved.
            world.registry.emplace<PositionHasChanged>(entity);
        }
    }
}

} // namespace Server
} // namespace AM
