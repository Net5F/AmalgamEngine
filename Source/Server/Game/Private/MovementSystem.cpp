#include "MovementSystem.h"
#include "World.h"
#include "Debug.h"

namespace AM
{
namespace Server
{

MovementSystem::MovementSystem(World& inWorld)
: world(inWorld)
, builder(BUILDER_BUFFER_SIZE)
{
}

void MovementSystem::processMovements(double deltaSeconds)
{
    for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        // TODO: Split this into "change inputs" and "add velocity based on current inputs".
        //       Then, Put the former behind an "isDirty" check.
        if ((world.componentFlags[entityID] & ComponentFlag::Input)
        && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
            // Process the input state for each entity.
            changeVelocity(entityID, world.inputs[entityID].inputStates, deltaSeconds);
        }

        /* Move all entities that have a position and movement component. */
        if ((world.componentFlags[entityID] & ComponentFlag::Position)
        && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
            // Update the positions based on the velocities.
            world.positions[entityID].x +=
                (deltaSeconds * world.movements[entityID].velX);
            world.positions[entityID].y +=
                (deltaSeconds * world.movements[entityID].velY);
        }

        /* Move the sprites to the new positions. */
        if ((world.componentFlags[entityID] & ComponentFlag::Position)
        && (world.componentFlags[entityID] & ComponentFlag::Sprite)) {
            world.sprites[entityID].posInWorld.x = world.positions[entityID].x;
            world.sprites[entityID].posInWorld.y = world.positions[entityID].y;
        }
    }
}

void MovementSystem::changeVelocity(
EntityID entityID,
std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates,
double deltaSeconds)
{
    MovementComponent& movement = world.movements[entityID];
    // TODO: Add movementSpeed to MovementComponent.
    // Constant acceleration.
    float acceleration = 750;

    // Handle up/down (favors up).
    if (inputStates[Input::Up] == Input::Pressed) {
        movement.velY -= (acceleration * deltaSeconds);

        if (movement.velY < -(movement.maxVelY)) {
            movement.velY = -(movement.maxVelY);
        }
    }
    else if (inputStates[Input::Down] == Input::Pressed) {
        movement.velY += (acceleration * deltaSeconds);

        if (movement.velY > movement.maxVelY) {
            movement.velY = movement.maxVelY;
        }
    }
    else {
//        // Slow the entity down.
//        if (movement.velY > 0) {
//            movement.velY -= (acceleration * deltaSeconds);
//        }
//        else if (movement.velY < 0) {
//            movement.velY += (acceleration * deltaSeconds);
//        }
        movement.velY = 0;
    }

    // Handle left/right (favors right).
    if (inputStates[Input::Left] == Input::Pressed) {
        movement.velX -= (acceleration * deltaSeconds);

        if (movement.velX < -(movement.maxVelX)) {
            movement.velX = -(movement.maxVelX);
        }
    }
    else if (inputStates[Input::Right] == Input::Pressed) {
        movement.velX += (acceleration * deltaSeconds);

        if (movement.velX > movement.maxVelX) {
            movement.velX = movement.maxVelX;
        }
    }
    else {
//        // Slow the entity down.
//        if (movement.velX > 0) {
//            movement.velX -= (acceleration * deltaSeconds);
//        }
//        else if (movement.velX < 0) {
//            movement.velX += (acceleration * deltaSeconds);
//        }
        movement.velX = 0;
    }
}

} // namespace Server
} // namespace AM
