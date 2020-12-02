#include "NetworkUpdateSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageTools.h"
#include "EntityUpdate.h"
#include "Log.h"

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
    std::vector<EntityID> dirtyEntities;
    for (EntityID i = 0; i < MAX_ENTITIES; ++i) {
        if (world.entityIsDirty[i]) {
            dirtyEntities.push_back(i);
        }
    }

    /* Update clients as necessary. */
    for (EntityID entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        if ((world.componentFlags[entityID] & ComponentFlag::Client)) {
            // Build an array to flag which entities need to be sent to this
            // client.
            std::array<bool, MAX_ENTITIES> entitiesToSend = {};

            // Add all the dirty entities.
            // TODO: Check if the entity is in our AOI before adding.
            for (EntityID dirtyEntityID : dirtyEntities) {
                entitiesToSend[dirtyEntityID] = true;
            }

            // If this entity had a drop, add it.
            // (It mispredicted, so it needs to know the actual state it's in.)
            if (world.clients[entityID].messageWasDropped) {
                entitiesToSend[entityID] = true;
                world.clients[entityID].messageWasDropped = false;
            }

            // Send the entity whatever data it needs.
            constructAndSendUpdate(entityID, entitiesToSend);
        }
    }

    // Mark any dirty entities as clean
    for (EntityID dirtyEntityID : dirtyEntities) {
        world.entityIsDirty[dirtyEntityID] = false;
    }
}

void NetworkUpdateSystem::constructAndSendUpdate(
    EntityID entityID, std::array<bool, MAX_ENTITIES>& entitiesToSend)
{
    /** Fill the vector of entities to send. */
    EntityUpdate entityUpdate{};
    ClientComponent& clientComponent = world.clients[entityID];
    for (EntityID i = 0; i < MAX_ENTITIES; ++i) {
        if (entitiesToSend[i]) {
            fillEntityData(i, entityUpdate.entities);
        }
    }

    /* If there are updates to send, send an update message. */
    if (entityUpdate.entities.size() > 0) {
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
        network.send(clientComponent.netID, messageBuffer,
                     entityUpdate.tickNum);
    }
}

void NetworkUpdateSystem::fillEntityData(EntityID entityID,
                                         std::vector<Entity>& entities)
{
    /* Fill the message with the latest PositionComponent, MovementComponent,
       and InputComponent data. */
    InputComponent& input = world.inputs[entityID];
    PositionComponent& position = world.positions[entityID];
    MovementComponent& movement = world.movements[entityID];

    Uint32 flags = (ComponentFlag::Position & ComponentFlag::Movement
                    & ComponentFlag::Input);

    // TEMP - Doing this until C++20 where we can emplace brace initializers.
    entities.push_back({entityID, flags, input, position, movement});

    //    LOG_INFO("Sending: (%f, %f), (%f, %f)", position.x, position.y,
    //    movement.velX, movement.velY);
}

} // namespace Server
} // namespace AM
