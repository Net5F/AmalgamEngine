#include "ClientUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Serialize.h"
#include "EntityUpdate.h"
#include "Input.h"
#include "Position.h"
#include "Movement.h"
#include "ClientSimData.h"
#include "InputHasChanged.h"
#include "Log.h"
#include "Profiler.h"
#include <algorithm>

namespace AM
{
namespace Server
{
ClientUpdateSystem::ClientUpdateSystem(Simulation& inSim, World& inWorld,
                                       Network& inNetwork)
: sim(inSim)
, world(inWorld)
, network(inNetwork)
{
}

void ClientUpdateSystem::sendClientUpdates()
{
    SCOPED_CPU_SAMPLE(sendClientUpdate);

    // Send clients the updated movement state of any nearby entities that
    // have moved.
    auto clientView{world.registry.view<ClientSimData>()};
    for (entt::entity clientEntity : clientView) {
        // Collect the entities that have updated state that is relevant to
        // this client.
        ClientSimData& client{clientView.get<ClientSimData>(clientEntity)};
        collectEntitiesToSend(client, clientEntity);

        // If there is updated state to send, send an update message.
        if (entitiesToSend.size() > 0) {
            sendEntityUpdate(client);
        }
    }

    // Mark any entities with dirty inputs as clean.
    world.registry.clear<InputHasChanged>();
}

void ClientUpdateSystem::collectEntitiesToSend(ClientSimData& client, entt::entity clientEntity)
{
    /* Collect the entities that need to be sent to the client. */
    // Clear the vector.
    entitiesToSend.clear();

    // Add all of the entities that just entered this client's AOI.
    entitiesToSend.insert(entitiesToSend.end(), client.entitiesThatEnteredAOI.begin(),
        client.entitiesThatEnteredAOI.end());

    // Add any entities in this client's AOI that have dirty inputs.
    for (entt::entity entityInAOI : client.entitiesInAOI) {
        if (world.registry.all_of<InputHasChanged>(entityInAOI)) {
            entitiesToSend.push_back(entityInAOI);
        }
    }

    // If the client entity had an input drop, add it.
    // (It mispredicted, so it needs to know the actual state it's in.)
    if (client.inputWasDropped) {
        entitiesToSend.push_back(clientEntity);
        client.inputWasDropped = false;
    }

    // Remove duplicates from the vector.
    std::sort(entitiesToSend.begin(), entitiesToSend.end());
    entitiesToSend.erase(std::unique(entitiesToSend.begin(),
        entitiesToSend.end()), entitiesToSend.end());

    // Clear entitiesThatEnteredAOI so they don't get added again next tick.
    client.entitiesThatEnteredAOI.clear();
}

void ClientUpdateSystem::sendEntityUpdate(ClientSimData& client)
{
    auto movementGroup{world.registry.group<Input, Position, Movement>()};
    EntityUpdate entityUpdate{};

    // Add the entities to the message.
    for (entt::entity entityToSend : entitiesToSend) {
        auto [input, position, movement]
            = movementGroup.get<Input, Position, Movement>(entityToSend);
        entityUpdate.entityStates.push_back(
            {entityToSend, input, position, movement});
    }

    // Finish filling the other fields.
    entityUpdate.tickNum = sim.getCurrentTick();

    // Send the message.
    network.serializeAndSend(client.netID, entityUpdate,
                             entityUpdate.tickNum);
}

} // namespace Server
} // namespace AM
