#include "NetworkOutputSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageUtil.h"
#include <iostream>

namespace AM
{
namespace Server
{

NetworkOutputSystem::NetworkOutputSystem(Game& inGame, World& inWorld, Network& inNetwork)
: game(inGame)
, world(inWorld)
, network(inNetwork)
, builder(BUILDER_BUFFER_SIZE)
, timeSinceTick(0.0f)
{
}

void NetworkOutputSystem::updateClients(float deltaSeconds)
{
    timeSinceTick += deltaSeconds;
    if (timeSinceTick < NETWORK_OUTPUT_TICK_INTERVAL_S) {
        // It's not yet time to process the game tick.
        return;
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

    timeSinceTick = 0;
}

void NetworkOutputSystem::broadcastEntity(EntityID entityID)
{
    // Prep the builder for a new message.
    builder.Clear();

    /* Fill a message with the updated PositionComponent, NetworkOutputComponent,
       and InputComponent data. */
    // Translate the inputs to fb's enum.
    InputComponent& input = world.inputs[entityID];

    fb::InputState fbInputStates[Input::Type::NumTypes];
    std::array<Input::State, Input::NumTypes>& playerInputStates =
        world.inputs[entityID].inputStates;

    for (uint8_t i = 0; i < Input::Type::NumTypes; ++i) {
        // Translate the Input::State enum to fb::InputState.
        fbInputStates[i] = MessageUtil::convertToFbInputState(playerInputStates[i]);
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

    // Build the MovementComponent.
    MovementComponent& movement = world.movements[entityID];
    flatbuffers::Offset<fb::MovementComponent> movementComponent =
        fb::CreateMovementComponent(builder, movement.velX, movement.velY,
            movement.maxVelX, movement.maxVelY);
    std::cout << "Sending@" << game.getCurrentTick() << ": (" << position.x << ", "
    << position.y << ")" << ", (" << movement.velX << ", " << movement.velY << ")"
    << std::endl;

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
        game.getCurrentTick(), entity);

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

} // namespace Server
} // namespace AM
