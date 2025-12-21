#include "MovementSyncSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
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
MovementSyncSystem::MovementSyncSystem(const SimulationContext& inSimContext)
: simulation{inSimContext.simulation}
, world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, updatedEntities{}
, entitiesToSend{}
, movementSyncObserver{}
{
    // Observe Input and MovementModifiers. Everything else can be handled 
    // client-side.
    movementSyncObserver.bind(world.registry);
    movementSyncObserver.on_update<Input>().on_update<MovementModifiers>();
}

void MovementSyncSystem::sendMovementUpdates()
{
    ZoneScoped;

    // TODO: We should measure this function and see if it's more performant to
    //       instead loop over all entities that moved on this frame and add
    //       their AOI entities to a map of update messages to be sent.

    // Push all the updated entities into a vector and sort them.
    updatedEntities.clear();
    for (entt::entity entity : movementSyncObserver) {
        updatedEntities.push_back(entity);
    }
    std::sort(updatedEntities.begin(), updatedEntities.end());
    movementSyncObserver.clear();

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
