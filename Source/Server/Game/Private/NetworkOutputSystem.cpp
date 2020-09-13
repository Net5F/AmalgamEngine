#include "NetworkOutputSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageUtil.h"
#include "Debug.h"

namespace AM
{
namespace Server
{

NetworkOutputSystem::NetworkOutputSystem(Game& inGame, World& inWorld, Network& inNetwork)
: game(inGame)
, world(inWorld)
, network(inNetwork)
, builder(BUILDER_BUFFER_SIZE)
{
}

void NetworkOutputSystem::sendClientUpdates()
{

    // Collect the dirty entities.
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
            // Clearing here because serializeEntity uses the builder.
            builder.Clear();

            // Check if this client needs the full world state.
            ClientComponent& clientComponent = world.clients.find(entityID)->second;
            std::vector<flatbuffers::Offset<fb::Entity>> entityVector;
            if (!clientComponent.isInitialized) {
                // We need to send the client all entities.
                for (EntityID unseenEntID = 0; unseenEntID < MAX_ENTITIES; ++unseenEntID) {
                    if (world.componentFlags[unseenEntID] & ComponentFlag::Sprite) {
                        entityVector.push_back(serializeEntity(unseenEntID));
                    }
                }
                clientComponent.isInitialized = true;
            }
            else {
                // We only need to update the client with dirty entities.
                for (EntityID dirtyEntID : dirtyEntities) {
                    entityVector.push_back(serializeEntity(dirtyEntID));
                }
            }

            /* If there are updates to send, send an update message. */
            if (entityVector.size() > 0) {
                // Build an EntityUpdate.
                auto serializedEntities = builder.CreateVector(entityVector);
                flatbuffers::Offset<fb::EntityUpdate> entityUpdate =
                    fb::CreateEntityUpdate(builder, serializedEntities);

                // Build a Message.
                fb::MessageBuilder messageBuilder(builder);
                messageBuilder.add_tickTimestamp(game.getCurrentTick());
                messageBuilder.add_content_type(fb::MessageContent::EntityUpdate);
                messageBuilder.add_content(entityUpdate.Union());
                flatbuffers::Offset<fb::Message> message = messageBuilder.Finish();
                builder.Finish(message);

                // Send the message to all connected clients.
                network.send(clientComponent.networkID,
                    Network::constructMessage(builder.GetSize(), builder.GetBufferPointer()));
            }
        }
    }

    // Clean any dirty entities.
    for (EntityID dirtyEntity : dirtyEntities) {
        world.entityIsDirty[dirtyEntity] = false;
    }
}

flatbuffers::Offset<AM::fb::Entity> NetworkOutputSystem::serializeEntity(
EntityID entityID)
{
    /* Fill the message with the latest PositionComponent, MovementComponent,
       and InputComponent data. */
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
//    DebugInfo("Sending: (%f, %f), (%f, %f)", position.x, position.y, movement.velX,
//        movement.velY);

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

    return entityBuilder.Finish();
}

} // namespace Server
} // namespace AM
