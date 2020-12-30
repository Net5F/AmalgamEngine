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
    auto movementGroup = world.registry.group<Input, Position, Movement>();
    ignore(movementGroup);
}

Timer timer;
double time1, time2, time3 = 0;
double tempTime1, tempTime2, tempTime3 = 0;
int timerCounter = 0;
void NetworkUpdateSystem::sendClientUpdates()
{
    // Collect the dirty entities.
    timer.updateSavedTime();
    auto movementGroup = world.registry.group<Input, Position, Movement>();
    std::vector<EntityStateRefs> dirtyEntities;
    for (entt::entity entity : movementGroup) {
        Input& input = movementGroup.get<Input>(entity);
        if (input.isDirty) {
            auto [position, movement] = movementGroup.get<Position, Movement>(entity);
            dirtyEntities.push_back({entity, input, position, movement});
        }
    }
    tempTime1 += timer.getDeltaSeconds(true);

    /* Update clients as necessary. */
    auto clientGroup = world.registry.group<ClientSimData>(entt::get<Position>);
    for (entt::entity entity : clientGroup) {
        // Center this entity's AoI on its current position.
        auto [client, clientPosition]
            = clientGroup.get<ClientSimData, Position>(entity);
        client.aoi.setCenter(clientPosition);

        /* Collect the entities that need to be sent to this client. */
        EntityUpdate entityUpdate{};

        timer.updateSavedTime();
        // Add all the dirty entities.
        for (EntityStateRefs& state : dirtyEntities) {
            // Check if the dirty entity is in this client's AOI before adding.
            if (client.aoi.contains(state.position)) {
                entityUpdate.entityStates.push_back(
                    {state.entity, state.input, state.position, state.movement});
            }
        }
        tempTime2 += timer.getDeltaSeconds(true);

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
                auto [input, position, movement] = movementGroup.get<Input, Position, Movement>(entity);
                entityUpdate.entityStates.push_back({entity, input, position, movement});
                client.messageWasDropped = false;
            }
        }

        /* Send the collected entities to this client. */
        timer.updateSavedTime();
        sendUpdate(client, entityUpdate);
        tempTime3 += timer.getDeltaSeconds(true);
    }

    if (tempTime1 > time1) {
        time1 = tempTime1;
    }
    if (tempTime2 > time2) {
        time2 = tempTime2;
    }
    if (tempTime3 > time3) {
        time3 = tempTime3;
    }
    tempTime1 = 0;
    tempTime2 = 0;
    tempTime3 = 0;

    if (timerCounter == 150) {
        LOG_INFO("Time1: %.6f, Time2: %.6f, Time3: %.6f", time1, time2, time3);
        time1 = 0;
        time2 = 0;
        time3 = 0;
        timerCounter = 0;
    }
    else {
        timerCounter++;
    }

    // Mark any dirty entities as clean
    for (EntityStateRefs& state : dirtyEntities) {
        state.input.isDirty = false;
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
