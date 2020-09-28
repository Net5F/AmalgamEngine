#include "NetworkUpdateSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "Debug.h"

namespace AM
{
namespace Server
{

NetworkUpdateSystem::NetworkUpdateSystem(Game& inGame, World& inWorld, Network& inNetwork)
: game(inGame)
, world(inWorld)
, network(inNetwork)
//, builder(BUILDER_BUFFER_SIZE)
{
}

void NetworkUpdateSystem::sendClientUpdates()
{
    // Collect the dirty entities so we don't need to re-find them for every client.
    std::vector<EntityID> dirtyEntities;
    dirtyEntities.reserve(MAX_ENTITIES);
    for (EntityID i = 0; i < MAX_ENTITIES; ++i) {
        if (world.entityIsDirty[i]) {
            dirtyEntities.push_back(i);
        }
    }

    /* Update clients as necessary. */
    for (EntityID entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        if ((world.componentFlags[entityID] & ComponentFlag::Client)) {
            // Send the entity whatever data it needs.
            constructAndSendUpdate(entityID, dirtyEntities);
        }
    }

    // Clean any dirty entities.
    for (EntityID dirtyEntity : dirtyEntities) {
        world.entityIsDirty[dirtyEntity] = false;
    }
}

void NetworkUpdateSystem::constructAndSendUpdate(EntityID entityID,
                                                 std::vector<EntityID>& dirtyEntities)
{
//    // Clearing here because serializeEntity uses the builder.
//    builder.Clear();
//
//    /** Fill the vector of entities to send. */
//    std::vector<flatbuffers::Offset<fb::Entity>> entityVector;
//    ClientComponent& clientComponent = world.clients.find(entityID)->second;
//    if (!clientComponent.isInitialized) {
//        // New client, we need to send it all relevant entities.
//        for (EntityID i = 0; i < MAX_ENTITIES; ++i) {
//            if ((i != entityID)
//                  && (world.componentFlags[i] & ComponentFlag::Client)) {
//                entityVector.push_back(serializeEntity(i));
//            }
//        }
//        clientComponent.isInitialized = true;
//    }
//    else {
//        // We only need to update the client with dirty entities.
//        for (EntityID dirtyEntID : dirtyEntities) {
//            entityVector.push_back(serializeEntity(dirtyEntID));
//        }
//    }
//
//    /* If there are updates to send, send an update message. */
//    if (entityVector.size() > 0) {
//        DebugInfo("Queueing message for entity: %u with tick: %u", entityID,
//                  game.getCurrentTick());
//        // Build an EntityUpdate.
//        auto serializedEntities = builder.CreateVector(entityVector);
//        flatbuffers::Offset<fb::EntityUpdate> entityUpdate =
//            fb::CreateEntityUpdate(builder, serializedEntities);
//
//        // Build a Message.
//        fb::MessageBuilder messageBuilder(builder);
//        messageBuilder.add_tickTimestamp(game.getCurrentTick());
//        messageBuilder.add_content_type(fb::MessageContent::EntityUpdate);
//        messageBuilder.add_content(entityUpdate.Union());
//        flatbuffers::Offset<fb::Message> message = messageBuilder.Finish();
//        builder.Finish(message);
//
//        // Send the message to all connected clients.
//        network.send(clientComponent.networkID,
//            Network::constructMessage(builder.GetBufferPointer(), builder.GetSize()));
//    }
}

//flatbuffers::Offset<AM::fb::Entity> NetworkUpdateSystem::serializeEntity(
//EntityID entityID)
//{
//    /* Fill the message with the latest PositionComponent, MovementComponent,
//       and InputComponent data. */
//    // Prepare the inputs.
//    fb::InputState fbInputStates[Input::Type::NumTypes];
//    std::array<Input::State, Input::NumTypes>& playerInputStates =
//        world.inputs[entityID].inputStates;
//
//    for (uint8_t i = 0; i < Input::Type::NumTypes; ++i) {
//        // Translate the Input::State enum to fb::InputState.
//        fbInputStates[i] = MessageUtil::convertToFbInputState(playerInputStates[i]);
//    }
//
//    flatbuffers::Offset<flatbuffers::Vector<fb::InputState>> inputVector =
//        builder.CreateVector(fbInputStates, Input::Type::NumTypes);
//
//    // Build the InputComponent.
//    flatbuffers::Offset<fb::InputComponent> inputComponent = fb::CreateInputComponent(
//        builder, inputVector);
//
//    // Build the PositionComponent.
//    PositionComponent& position = world.positions[entityID];
//    flatbuffers::Offset<fb::PositionComponent> positionComponent =
//        fb::CreatePositionComponent(builder, position.x, position.y);
//
//    // Build the MovementComponent.
//    MovementComponent& movement = world.movements[entityID];
//    flatbuffers::Offset<fb::MovementComponent> movementComponent =
//        fb::CreateMovementComponent(builder, movement.velX, movement.velY,
//            movement.maxVelX, movement.maxVelY);
////    DebugInfo("Sending: (%f, %f), (%f, %f)", position.x, position.y, movement.velX,
////        movement.velY);
//
//    // Build the Entity.
//    auto entityName = builder.CreateString(world.entityNames[entityID]);
//    fb::EntityBuilder entityBuilder(builder);
//    entityBuilder.add_id(entityID);
//    entityBuilder.add_name(entityName);
//
//    // Mark the components that we're sending.
//    entityBuilder.add_flags(
//        ComponentFlag::Position & ComponentFlag::Movement & ComponentFlag::Input);
//    entityBuilder.add_positionComponent(positionComponent);
//    entityBuilder.add_movementComponent(movementComponent);
//    entityBuilder.add_inputComponent(inputComponent);
//
//    return entityBuilder.Finish();
//}

} // namespace Server
} // namespace AM
