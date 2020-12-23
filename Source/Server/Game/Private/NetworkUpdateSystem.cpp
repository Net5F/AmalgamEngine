#include "NetworkUpdateSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageTools.h"
#include "EntityUpdate.h"
#include "Input.h"
#include "ClientState.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
namespace Server
{
NetworkUpdateSystem::NetworkUpdateSystem(Game& inGame, World& inWorld,
                                         Network& inNetwork)
: game(inGame)
, world(inWorld)
, network(inNetwork)
{
}

void NetworkUpdateSystem::sendClientUpdates()
{
    // Collect the dirty entities.
    entt::registry& registry = world.registry;
    auto inputView = registry.view<Input>();
    std::vector<entt::entity> dirtyEntities;
    for (entt::entity entity : inputView) {
        if (inputView.get<Input>(entity).isDirty) {
            dirtyEntities.push_back(entity);
        }
    }

    /* Update clients as necessary. */
    auto clientView = registry.view<ClientState>();
    for (entt::entity entity : clientView) {
        // Collect the entities that need to be sent to this client.
        // Uses an array since we're adding in multiple passes and want unique
        // elements.
        std::vector<entt::entity> entitiesToSend;

        // Add all the dirty entities.
        // TODO: Check if the entity is in our AOI before adding.
        for (entt::entity dirtyEntityID : dirtyEntities) {
            entitiesToSend.push_back(dirtyEntityID);
        }

        // If this entity had a drop, add it.
        // (It mispredicted, so it needs to know the actual state it's in.)
        ClientState& client = clientView.get<ClientState>(entity);
        if (client.messageWasDropped) {
            // Only add unique entities.
            if (std::find(entitiesToSend.begin(), entitiesToSend.end(), entity)
            != entitiesToSend.end()) {
                entitiesToSend.push_back(entity);
                client.messageWasDropped = false;
            }
        }

        // Send the entity whatever data it needs.
        constructAndSendUpdate(entity, entitiesToSend);
    }

    // Mark any dirty entities as clean
    for (entt::entity dirtyEntity : dirtyEntities) {
        inputView.get<Input>(dirtyEntity).isDirty = false;
    }
}

void NetworkUpdateSystem::constructAndSendUpdate(
    entt::entity entity, std::vector<entt::entity>& entitiesToSend)
{
    /** Fill the vector of entities to send. */
    EntityUpdate entityUpdate{};
    auto clientView = world.registry.view<ClientState>();
    ClientState& client = clientView.get<ClientState>(entity);
    for (entt::entity entity : entitiesToSend) {
        fillEntityData(entity, entityUpdate.entityStates);
    }

    /* If there are updates to send, send an update message. */
    if (entityUpdate.entityStates.size() > 0) {
        // Finish filling the EntityUpdate.
        entityUpdate.tickNum = game.getCurrentTick();

        // Serialize the EntityUpdate.
        BinaryBufferSharedPtr messageBuffer
            = std::make_shared<BinaryBuffer>(Peer::MAX_MESSAGE_SIZE);
        std::size_t messageSize = MessageTools::serialize(
            *messageBuffer, entityUpdate, MESSAGE_HEADER_SIZE);

        // Fill the buffer with the appropriate message header.
        MessageTools::fillMessageHeader(MessageType::EntityUpdate, messageSize,
                                        messageBuffer, 0);

        // Send the message.
        network.send(client.netID, messageBuffer, entityUpdate.tickNum);
    }
}

void NetworkUpdateSystem::fillEntityData(entt::entity entity,
                                         std::vector<EntityState>& entityStates)
{
    /* Fill the message with the latest PositionComponent, MovementComponent,
       and InputComponent data. */
    entt::registry& registry = world.registry;
    Input& input = registry.get<Input>(entity);
    Position& position = registry.get<Position>(entity);
    Movement& movement = registry.get<Movement>(entity);

    // TEMP - Doing this until C++20 where we can emplace brace initializers.
    entityStates.push_back({entity, input, position, movement});

    //    LOG_INFO("Sending: (%f, %f), (%f, %f)", position.x, position.y,
    //    movement.velX, movement.velY);
}

} // namespace Server
} // namespace AM
