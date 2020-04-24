#include "MovementSystem.h"
#include "World.h"
#include "NetworkServer.h"

using namespace AM;

AM::MovementSystem::MovementSystem(World& inWorld, NetworkServer& inNetwork)
: world(inWorld)
, network(inNetwork)
, builder(BUILDER_BUFFER_SIZE)
{
}

void AM::MovementSystem::processMovements()
{
    for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        // If this entity doesn't have dirty inputs, it doesn't need to be
        if (!(world.entityIsDirty[entityID])) {
            continue;
        }

        /* Process input state on any dirty entity that has an input component
           and a movement component. */
        if ((world.componentFlags[entityID] & ComponentFlag::Input)
        && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
            // Process the input state for each entity.
            changeVelocity(entityID, world.inputs[entityID].inputStates);
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

    /* Send all updated entity states to all clients. */
    for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        // Only send updates for entities that changed.
        if (world.entityIsDirty[entityID]) {
            broadcastEntity(entityID);
        }
    }
}

void AM::MovementSystem::changeVelocity(
EntityID entityID,
std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates)
{
    MovementComponent& movement = world.movements[entityID];
    // Handle up/down (favors up).
    if (inputStates[Input::Up] == Input::Pressed) {
        movement.velY -= 0.25;

        if (movement.velY < movement.maxVelY) {
            movement.velY = -(movement.maxVelY);
        }
    }
    else if (inputStates[Input::Down] == Input::Pressed) {
        movement.velY += 0.25;

        if (movement.velY > movement.maxVelY) {
            movement.velY = movement.maxVelY;
        }
    }
    else {
        // Slow the entity down.
        if (movement.velY > 0) {
            movement.velY -= 0.25;
        }
        else if (movement.velY < 0) {
            movement.velY += 0.25;
        }
    }

    // Handle left/right (favors right).
    if (inputStates[Input::Left] == Input::Pressed) {
        movement.velX -= 0.25;

        if (movement.velX < movement.maxVelX) {
            movement.velX = -(movement.maxVelX);
        }
    }
    else if (inputStates[Input::Right] == Input::Pressed) {
        movement.velX += 0.25;

        if (movement.velX > movement.maxVelX) {
            movement.velX = movement.maxVelX;
        }
    }
    else {
        // Slow the entity down.
        if (movement.velX > 0) {
            movement.velX -= 0.25;
        }
        else if (movement.velX < 0) {
            movement.velX += 0.25;
        }
    }
}

void AM::MovementSystem::broadcastEntity(EntityID entityID)
{
    /* Fill a message with the updated PositionComponent, MovementComponent,
       and SpriteComponent data. */
    // Build the PositionComponent.
    PositionComponent& position = world.positions[entityID];
    flatbuffers::Offset<fb::PositionComponent> positionComponent =
        fb::CreatePositionComponent(builder, position.x, position.y);

    // Build the MovementComponent.
    MovementComponent& movement = world.movements[entityID];
    flatbuffers::Offset<fb::MovementComponent> movementComponent =
        fb::CreateMovementComponent(builder, movement.velX, movement.velY,
            movement.maxVelX, movement.maxVelY);

    // Build the Entity.
    auto entityName = builder.CreateString(world.entityNames[entityID]);
    fb::EntityBuilder entityBuilder(builder);
    entityBuilder.add_id(entityID);
    entityBuilder.add_name(entityName);

    // Mark that we only are sending the InputComponent.
    entityBuilder.add_flags(ComponentFlag::Position & ComponentFlag::Movement);
    entityBuilder.add_positionComponent(positionComponent);
    entityBuilder.add_movementComponent(movementComponent);

    std::vector<flatbuffers::Offset<fb::Entity>> entityVector;
    entityVector.push_back(entityBuilder.Finish());
    auto entity = builder.CreateVector(entityVector);

    // Build an EntityUpdate.
    flatbuffers::Offset<fb::EntityUpdate> entityUpdate = fb::CreateEntityUpdate(builder,
        entity);

    // Build a Message.
    fb::MessageBuilder messageBuilder(builder);
    messageBuilder.add_content_type(fb::MessageContent::EntityUpdate);
    messageBuilder.add_content(entityUpdate.Union());
    flatbuffers::Offset<fb::Message> message = messageBuilder.Finish();
    builder.Finish(message);

    // Send the message to all connected clients.
    Uint8* buffer = builder.GetBufferPointer();
    network.sendToAll(
        std::make_shared<std::vector<Uint8>>(buffer, (buffer + builder.GetSize())));
}
