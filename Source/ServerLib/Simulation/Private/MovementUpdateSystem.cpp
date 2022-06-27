#include "MovementUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Serialize.h"
#include "MovementUpdate.h"
#include "Input.h"
#include "Position.h"
#include "Velocity.h"
#include "ClientSimData.h"
#include "InputHasChanged.h"
#include "Log.h"
#include "Tracy.hpp"
#include <algorithm>

namespace AM
{
namespace Server
{
MovementUpdateSystem::MovementUpdateSystem(Simulation& inSimulation,
                                           World& inWorld, Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
{
}

void MovementUpdateSystem::sendMovementUpdates()
{
    ZoneScoped;

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

void MovementUpdateSystem::collectEntitiesToSend(ClientSimData& client,
                                                 entt::entity clientEntity)
{
    /* Collect the entities that need to be sent to the client. */
    // Clear the vector.
    entitiesToSend.clear();

    // Add all of the entities that just entered this client's AOI.
    entitiesToSend.insert(entitiesToSend.end(),
                          client.entitiesThatEnteredAOI.begin(),
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
    // Else if the client entity has dirty inputs, add it.
    else if (world.registry.all_of<InputHasChanged>(clientEntity)) {
        entitiesToSend.push_back(clientEntity);
    }

    // Remove duplicates from the vector.
    std::sort(entitiesToSend.begin(), entitiesToSend.end());
    entitiesToSend.erase(
        std::unique(entitiesToSend.begin(), entitiesToSend.end()),
        entitiesToSend.end());

    // Clear entitiesThatEnteredAOI so they don't get added again next tick.
    client.entitiesThatEnteredAOI.clear();
}

void MovementUpdateSystem::sendEntityUpdate(ClientSimData& client)
{
    auto movementGroup{world.registry.group<Input, Position, Velocity>()};
    MovementUpdate movementUpdate{};

    // Add the entities to the message.
    for (entt::entity entityToSend : entitiesToSend) {
        auto [input, position, velocity]
            = movementGroup.get<Input, Position, Velocity>(entityToSend);
        movementUpdate.movementStates.push_back(
            {entityToSend, input, position, velocity});
    }

    // Finish filling the other fields.
    movementUpdate.tickNum = simulation.getCurrentTick();

    // Send the message.
    network.serializeAndSend(client.netID, movementUpdate,
                             movementUpdate.tickNum);
}

} // namespace Server
} // namespace AM
