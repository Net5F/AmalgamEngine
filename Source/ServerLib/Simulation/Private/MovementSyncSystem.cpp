#include "MovementSyncSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "MovementUpdate.h"
#include "EnttGroups.h"
#include "ClientSimData.h"
#include "MovementModifiers.h"
#include "Log.h"
#include "tracy/Tracy.hpp"
#include <algorithm>

namespace AM
{
namespace Server
{
MovementSyncSystem::MovementSyncSystem(Simulation& inSimulation, World& inWorld,
                                       Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, updatedEntities{}
, entitiesToSend{}
, inputObserver{world.registry,
                entt::collector.update<Input>().update<MovementModifiers>()}
{
}

void MovementSyncSystem::sendMovementUpdates()
{
    ZoneScoped;

    // TODO: We should measure this function and see if it's more performant to
    //       instead loop over all entities that moved on this frame and add
    //       their AOI entities to a map of update messages to be sent.

    // Push all the updated entities into a vector and sort them.
    updatedEntities.clear();
    for (entt::entity entity : inputObserver) {
        updatedEntities.push_back(entity);
    }
    std::sort(updatedEntities.begin(), updatedEntities.end());
    inputObserver.clear();

    // Send clients the updated movement state of any nearby entities that have
    // changed inputs, teleported, etc.
    auto clientView{world.registry.view<ClientSimData>()};
    for (auto [clientEntity, client] : clientView.each()) {
        // Collect the entities that have updated state that is relevant to
        // this client.
        collectEntitiesToSend(client);

        // If there is updated state to send, send an update message.
        if (entitiesToSend.size() > 0) {
            sendEntityUpdate(client);
        }
    }
}

void MovementSyncSystem::collectEntitiesToSend(ClientSimData& client)
{
    /* Collect the entities that need to be sent to the client. */
    // Clear the vector.
    entitiesToSend.clear();

    // Fill entitiesToSend with all of the entities that are both updated and
    // in this client's AOI.
    std::set_intersection(updatedEntities.begin(), updatedEntities.end(),
                          client.entitiesInAOI.begin(),
                          client.entitiesInAOI.end(),
                          std::back_inserter(entitiesToSend));
}

void MovementSyncSystem::sendEntityUpdate(ClientSimData& client)
{
    auto movementGroup{EnttGroups::getMovementGroup(world.registry)};
    MovementUpdate movementUpdate{};

    // Add the entities to the message.
    for (entt::entity entityToSend : entitiesToSend) {
        auto [input, position, movement, movementMods]
            = movementGroup.get<Input, Position, Movement, MovementModifiers>(
                entityToSend);
        movementUpdate.movementStates.push_back(
            {entityToSend, input, position, movement, movementMods});
    }

    // Finish filling the other fields.
    movementUpdate.tickNum = simulation.getCurrentTick();

    // Send the message.
    network.serializeAndSend(client.netID, movementUpdate,
                             movementUpdate.tickNum);
}

} // namespace Server
} // namespace AM
