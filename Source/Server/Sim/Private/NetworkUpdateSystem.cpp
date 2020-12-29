#include "NetworkUpdateSystem.h"
#include "Sim.h"
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
NetworkUpdateSystem::NetworkUpdateSystem(Sim& inSim, World& inWorld,
                                         Network& inNetwork)
: sim(inSim)
, world(inWorld)
, network(inNetwork)
{
    // Init the groups that we'll be using.
    auto clientGroup = world.registry.group<ClientSimData>(entt::get<Position>);
    ignore(clientGroup);
    auto dirtyGroup = world.registry.group<Input, Position, Movement>();
    ignore(dirtyGroup);
}

Timer timer;
double time1, time2 = 0;
int timerCounter = 0;
void NetworkUpdateSystem::sendClientUpdates()
{
    // Collect the dirty entities.
    auto dirtyGroup = world.registry.group<Input, Position, Movement>();
    std::vector<EntityState> dirtyEntities;
    for (entt::entity dirtyEntity : dirtyGroup) {
        auto [input, position, movement] = dirtyGroup.get(dirtyEntity);
        if (input.isDirty) {
            dirtyEntities.push_back({dirtyEntity, input, position, movement});
        }
    }

    /* Update clients as necessary. */
    auto clientGroup = world.registry.group<ClientSimData>(entt::get<Position>);
    double tempTime1 = 0;
    double tempTime2 = 0;
    for (entt::entity entity : clientGroup) {
        // Center this entity's AoI on its current position.
        auto [client, clientPosition]
            = clientGroup.get<ClientSimData, Position>(entity);
        client.aoi.setCenter(clientPosition);

        /* Collect the entities that need to be sent to this client. */
        EntityUpdate entityUpdate{};

        // Add all the dirty entities.
        timer.updateSavedTime();
        for (EntityState& entityState : dirtyEntities) {
            // Check if the dirty entity is in this client's AOI before adding.
            if (client.aoi.contains(entityState.position)) {
                entityUpdate.entityStates.push_back(entityState);
            }
        }
        tempTime1 += timer.getDeltaSeconds(false);

        // If this entity had a drop, add it.
        // (It mispredicted, so it needs to know the actual state it's in.)
        if (client.messageWasDropped) {
            // Only add entities if they will be unique.
            bool playerFound = false;
            for (EntityState& entityState : entityUpdate.entityStates) {
                if (entityState.entity == entity) {
                    playerFound = true;
                }
            }
            if (!playerFound) {
                auto [input, position, movement] = world.registry.get<Input, Position, Movement>(entity);
                entityUpdate.entityStates.push_back({entity, input, position, movement});
                client.messageWasDropped = false;
            }
        }

        /* Send the collected entities to this client. */
        timer.updateSavedTime();
        sendUpdate(client, entityUpdate);
        tempTime2 += timer.getDeltaSeconds(false);
    }

    if (tempTime1 > time1) {
        time1 = tempTime1;
    }
    if (tempTime2 > time2) {
        time2 = tempTime2;
    }

    if (timerCounter == 150) {
        LOG_INFO("Time1: %.6f, Time2: %.6f", time1, time2);
        time1 = 0;
        time2 = 0;
        timerCounter = 0;
    }
    else {
        timerCounter++;
    }

    // Mark any dirty entities as clean
    for (entt::entity dirtyEntity : dirtyGroup) {
        Input& input = dirtyGroup.get<Input>(dirtyEntity);
        input.isDirty = false;
    }
}

void NetworkUpdateSystem::sendUpdate(
    ClientSimData& client, EntityUpdate& entityUpdate)
{
    /* If there are updates to send, send an update message. */
    if (entityUpdate.entityStates.size() > 0) {
        // Finish filling the EntityUpdate.
        entityUpdate.tickNum = sim.getCurrentTick();

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

} // namespace Server
} // namespace AM
