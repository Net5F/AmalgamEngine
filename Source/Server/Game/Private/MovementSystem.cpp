#include "MovementSystem.h"
#include "World.h"
#include "Network.h"

namespace AM
{
namespace Server
{

MovementSystem::MovementSystem(World& inWorld, Network& inNetwork)
: world(inWorld)
, network(inNetwork)
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

    /* Send all updated entity states to all clients. */
    for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        // Only send updates for entities that changed.
        if (world.entityIsDirty[entityID]) {
            std::cout << "Broadcasting: " << entityID << std::endl;
            broadcastEntity(entityID);
            world.entityIsDirty[entityID] = false;
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

void MovementSystem::broadcastEntity(EntityID entityID)
{
    // Prep the builder for a new message.
    builder.Clear();

    /* Fill a message with the updated PositionComponent, MovementComponent,
       and InputComponent data. */
    // Translate the inputs to fb's enum.
    InputComponent& input = world.inputs[entityID];

    fb::InputState fbInputStates[Input::Type::NumTypes];
    std::array<Input::State, Input::NumTypes>& playerInputStates =
        world.inputs[entityID].inputStates;

    for (uint8_t i = 0; i < Input::Type::NumTypes; ++i) {
        // Translate the Input::State enum to fb::InputState.
        fbInputStates[i] = convertToFbInputState(playerInputStates[i]);
    }

    flatbuffers::Offset<flatbuffers::Vector<fb::InputState>> inputVector =
        builder.CreateVector(fbInputStates, Input::Type::NumTypes);

    // Build the InputComponent.
    flatbuffers::Offset<fb::InputComponent> inputComponent = fb::CreateInputComponent(
        builder, inputVector);

    // Build the PositionComponent.
    PositionComponent& position = world.positions[entityID];
    flatbuffers::Offset<fb::PositionComponent> positionComponent =
        fb::CreatePositionComponent(builder, position.x, position.y);
    std::cout << "Sending: (" << position.x << ", " << position.y << ")" << std::endl;

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

    // Mark the components that we're sending.
    entityBuilder.add_flags(
        ComponentFlag::Position & ComponentFlag::Movement & ComponentFlag::Input);
    entityBuilder.add_positionComponent(positionComponent);
    entityBuilder.add_movementComponent(movementComponent);
    entityBuilder.add_inputComponent(inputComponent);

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

fb::InputState MovementSystem::convertToFbInputState(Input::State state)
{
    switch (state)
    {
        case Input::Invalid:
            return fb::InputState::Invalid;
            break;
        case Input::Pressed:
            return fb::InputState::Pressed;
            break;
        case Input::Released:
            return fb::InputState::Released;
            break;
        default:
            return fb::InputState::Invalid;
    }
}

} // namespace Server
} // namespace AM
