#include "NetworkUpdateSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageTools.h"
#include "EntityUpdate.h"
#include "Input.h"
#include "Position.h"
#include "ClientSimData.h"
#include "Ignore.h"
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
    // Init the groups that we'll be using.
    auto clientGroup = world.registry.group<ClientSimData>(entt::get<Position>);
    ignore(clientGroup);
    auto dirtyGroup = world.registry.group<Input, Position>();
    ignore(dirtyGroup);
}

void NetworkUpdateSystem::sendClientUpdates()
{
    // Collect the dirty entities.
    auto dirtyGroup = world.registry.group<Input, Position>();
    std::vector<entt::entity> dirtyEntities;
    for (entt::entity entity : dirtyGroup) {
        if (dirtyGroup.get<Input>(entity).isDirty) {
            dirtyEntities.push_back(entity);
        }
    }

    /* Update clients as necessary. */
    auto clientGroup = world.registry.group<ClientSimData>(entt::get<Position>);
    for (entt::entity entity : clientGroup) {
        // Center this entity's AoI on its current position.
        auto [client, position] = clientGroup.get<ClientSimData, Position>(entity);
        client.aoi.setCenter(position);

        /* Collect the entities that need to be sent to this client. */
        std::vector<entt::entity> entitiesToSend;

        // Add all the dirty entities.
        for (entt::entity dirtyEntity : dirtyEntities) {
            // Check if the dirty entity is in this client's AOI before adding.
            Position& position = dirtyGroup.get<Position>(dirtyEntity);
            if (client.aoi.contains(position)) {
                entitiesToSend.push_back(dirtyEntity);
            }
        }

        // If this entity had a drop, add it.
        // (It mispredicted, so it needs to know the actual state it's in.)
        if (client.messageWasDropped) {
            // Only add entities if they will be unique.
            if (std::find(entitiesToSend.begin(), entitiesToSend.end(), entity)
                != entitiesToSend.end()) {
                entitiesToSend.push_back(entity);
                client.messageWasDropped = false;
            }
        }

        /* Send the collected entities to this client. */
        constructAndSendUpdate(entity, entitiesToSend);
    }

    // Mark any dirty entities as clean
    for (entt::entity dirtyEntity : dirtyEntities) {
        dirtyGroup.get<Input>(dirtyEntity).isDirty = false;
    }
}

void NetworkUpdateSystem::constructAndSendUpdate(
    entt::entity entity, std::vector<entt::entity>& entitiesToSend)
{
    /** Fill the vector of entities to send. */
    EntityUpdate entityUpdate{};
    auto clientView = world.registry.view<ClientSimData>();
    ClientSimData& client = clientView.get<ClientSimData>(entity);
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
    auto [input, position, movement] = registry.get<Input, Position, Movement>(entity);

    // TEMP - Doing this until C++20 where we can emplace brace initializers.
    entityStates.push_back(
        {registry.entity(entity), input, position, movement});

    //    LOG_INFO("Sending: (%f, %f), (%f, %f)", position.x, position.y,
    //    movement.velX, movement.velY);
}

} // namespace Server
} // namespace AM
