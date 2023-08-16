#include "MovementSyncSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Serialize.h"
#include "MovementUpdate.h"
#include "Input.h"
#include "Position.h"
#include "Velocity.h"
#include "Rotation.h"
#include "Collision.h"
#include "ClientSimData.h"
#include "MovementStateNeedsSync.h"
#include "Log.h"
#include "Tracy.hpp"
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
{
}

void MovementSyncSystem::sendMovementUpdates()
{
    ZoneScoped;

    // Send clients the updated movement state of any nearby entities that have
    // changed inputs, teleported, etc.
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

    // Clear the sync flags from every entity, since we just handled them.
    world.registry.clear<MovementStateNeedsSync>();
}

void MovementSyncSystem::collectEntitiesToSend(ClientSimData& client,
                                               entt::entity clientEntity)
{
    /* Collect the entities that need to be sent to the client. */
    // Clear the vector.
    entitiesToSend.clear();

    // Add any entities in this client's AOI that need to be synced.
    for (entt::entity entityInAOI : client.entitiesInAOI) {
        if (world.registry.all_of<MovementStateNeedsSync>(entityInAOI)) {
            entitiesToSend.push_back(entityInAOI);
        }
    }

    // If the client entity itself needs to be synced, add it.
    if (world.registry.all_of<MovementStateNeedsSync>(clientEntity)) {
        entitiesToSend.push_back(clientEntity);
    }

    // Remove duplicates from the vector.
    std::sort(entitiesToSend.begin(), entitiesToSend.end());
    entitiesToSend.erase(
        std::unique(entitiesToSend.begin(), entitiesToSend.end()),
        entitiesToSend.end());
}

void MovementSyncSystem::sendEntityUpdate(ClientSimData& client)
{
    auto movementGroup = world.registry.group<Input, Position, PreviousPosition,
                                              Velocity, Rotation, Collision>();
    MovementUpdate movementUpdate{};

    // Add the entities to the message.
    for (entt::entity entityToSend : entitiesToSend) {
        auto [input, position, velocity, rotation]
            = movementGroup.get<Input, Position, Velocity, Rotation>(
                entityToSend);
        movementUpdate.movementStates.push_back(
            {entityToSend, input, position, velocity, rotation});
    }

    // Finish filling the other fields.
    movementUpdate.tickNum = simulation.getCurrentTick();

    // Send the message.
    network.serializeAndSend(client.netID, movementUpdate,
                             movementUpdate.tickNum);
}

} // namespace Server
} // namespace AM
