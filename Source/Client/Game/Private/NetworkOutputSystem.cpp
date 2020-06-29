#include "NetworkOutputSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageUtil.h"
#include "NetworkHelpers.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

NetworkOutputSystem::NetworkOutputSystem(Game& inGame, World& inWorld, Network& inNetwork)
: game(inGame)
, world(inWorld)
, network(inNetwork)
, builder(BUILDER_BUFFER_SIZE)
{
}

void NetworkOutputSystem::sendInputState()
{
    /* Send the updated state to the server. */
    // Prep the builder for a new message.
    builder.Clear();

    // Create the vector of entity data.
    std::vector<flatbuffers::Offset<fb::Entity>> entityVector;
    EntityID playerID = world.playerID;
    if (world.playerIsDirty) {
        // Only send new data if we've changed. If we haven't, we'll still send the empty
        // message as a heartbeat.
        // TODO: Don't send a message if we haven't changed, let the network do the heartbeat.
        entityVector.push_back(serializeEntity(playerID));
    }
    auto serializedEntity = builder.CreateVector(entityVector);

    // Build an EntityUpdate.
    flatbuffers::Offset<fb::EntityUpdate> entityUpdate = fb::CreateEntityUpdate(builder,
        serializedEntity);

    // Build a Message.
    Uint32 currentTick = game.getCurrentTick();
    fb::MessageBuilder messageBuilder(builder);
    messageBuilder.add_tickTimestamp(currentTick);
    messageBuilder.add_content_type(fb::MessageContent::EntityUpdate);
    messageBuilder.add_content(entityUpdate.Union());
    flatbuffers::Offset<fb::Message> message = messageBuilder.Finish();
    builder.Finish(message);

    // Send the message.
    network.send(
        NetworkHelpers::constructMessage(builder.GetSize(), builder.GetBufferPointer()));

    world.playerIsDirty = false;
}

flatbuffers::Offset<AM::fb::Entity> NetworkOutputSystem::serializeEntity(
EntityID playerID)
{
    // Translate the inputs to fb's enum.
    fb::InputState fbInputStates[Input::Type::NumTypes];
    std::array<Input::State, Input::NumTypes>& playerInputStates =
        world.inputs[playerID].inputStates;
    for (uint8_t i = 0; i < Input::Type::NumTypes; ++i) {
        // Translate the Input::State enum to fb::InputState.
        fbInputStates[i] = MessageUtil::convertToFbInputState(playerInputStates[i]);
    }
    flatbuffers::Offset<flatbuffers::Vector<fb::InputState>> inputVector =
        builder.CreateVector(fbInputStates, Input::Type::NumTypes);

    // Build the inputComponent.
    flatbuffers::Offset<fb::InputComponent> inputComponent = fb::CreateInputComponent(
        builder, inputVector);

    // Build the Entity.
    auto entityName = builder.CreateString(world.entityNames[playerID]);
    fb::EntityBuilder entityBuilder(builder);
    entityBuilder.add_id(playerID);
    entityBuilder.add_name(entityName);

    // Mark that we only are sending the InputComponent.
    entityBuilder.add_flags(ComponentFlag::Input);
    entityBuilder.add_inputComponent(inputComponent);

    return entityBuilder.Finish();
}

} // namespace Client
} // namespace AM
