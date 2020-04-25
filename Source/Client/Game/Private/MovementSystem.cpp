#include "MovementSystem.h"
#include "World.h"


AM::MovementSystem::MovementSystem(World& inWorld)
: world(inWorld)
{
}

void AM::MovementSystem::processMovements(double deltaMs)
{
    for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        /* Process input state on everything that has an input component and a movement component. */
        if ((world.componentFlags[entityID] & ComponentFlag::Input)
        && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
            // Process the input state for each entity.
            changeVelocity(entityID, world.inputs[entityID].inputStates, deltaMs);
        }

        /* Move all entities that have a position and movement component. */
        if ((world.componentFlags[entityID] & ComponentFlag::Position)
        && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
            // Update the positions based on the velocities.
            world.positions[entityID].x += world.movements[entityID].velX;
            world.positions[entityID].y += world.movements[entityID].velY;
        }

        /* Move the sprites to the new positions. */
        if ((world.componentFlags[entityID] & ComponentFlag::Position)
        && (world.componentFlags[entityID] & ComponentFlag::Sprite)) {
            world.sprites[entityID].posInWorld.x = world.positions[entityID].x;
            world.sprites[entityID].posInWorld.y = world.positions[entityID].y;
        }
    }
}

void AM::MovementSystem::changeVelocity(
EntityID entityID,
std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates,
double deltaMs)
{
    MovementComponent& movement = world.movements[entityID];
    // TODO: Add movement speed to the component.
    float movementSpeed = 5;

    // Handle up/down (favors up).
    if (inputStates[Input::Up] == Input::Pressed) {
        movement.velY -= (movementSpeed * deltaMs);

        if (movement.velY < movement.maxVelY) {
            movement.velY = -(movement.maxVelY);
        }
    }
    else if (inputStates[Input::Down] == Input::Pressed) {
        movement.velY += (movementSpeed * deltaMs);

        if (movement.velY > movement.maxVelY) {
            movement.velY = movement.maxVelY;
        }
    }
    else {
//        // Slow the entity down.
//        if (movement.velY > 0) {
//            movement.velY -= (movementSpeed * deltaMs);
//        }
//        else if (movement.velY < 0) {
//            movement.velY += (movementSpeed * deltaMs);
//        }
        movement.velY = 0;
    }

    // Handle left/right (favors right).
    if (inputStates[Input::Left] == Input::Pressed) {
        movement.velX -= (movementSpeed * deltaMs);

        if (movement.velX < movement.maxVelX) {
            movement.velX = -(movement.maxVelX);
        }
    }
    else if (inputStates[Input::Right] == Input::Pressed) {
        movement.velX += (movementSpeed * deltaMs);

        if (movement.velX > movement.maxVelX) {
            movement.velX = movement.maxVelX;
        }
    }
    else {
//        // Slow the entity down.
//        if (movement.velX > 0) {
//            movement.velX -= (movementSpeed * deltaMs);
//        }
//        else if (movement.velX < 0) {
//            movement.velX += (movementSpeed * deltaMs);
//        }
        movement.velX = 0;
    }
}
