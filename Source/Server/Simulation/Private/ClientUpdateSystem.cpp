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

    auto clientView{world.registry.view<ClientSimData, Position>()};
    auto dirtyView{world.registry.view<InputHasChanged>()};
    auto movementGroup{world.registry.group<Input, Position, Movement>()};

    // Update clients as necessary.
    for (entt::entity entity : clientView) {
        /* Collect the entities that need to be sent to this client. */
        auto [client, clientPosition]
            = clientView.get<ClientSimData, Position>(entity);
        EntityUpdate entityUpdate{};

        // If any entities in this client's AOI have dirty inputs, add them
        // to the message.
        for (entt::entity entityInAOI : client.entitiesInAOI) {
            if (world.registry.all_of<InputHasChanged>(entityInAOI)) {
                auto [input, position, movement]
                    = movementGroup.get<Input, Position, Movement>(entityInAOI);
                entityUpdate.entityStates.push_back({entityInAOI, input, position, movement});
            }
        }

        // If this entity had an input drop, add it.
        // (It mispredicted, so it needs to know the actual state it's in.)
        if (client.inputWasDropped) {
            // Only add the player entity if it isn't already included.
            bool playerFound = false;
            for (EntityState& entityState : entityUpdate.entityStates) {
                if (entityState.entity == entity) {
                    playerFound = true;
                }
            }
            if (!playerFound) {
                auto [input, position, movement]
                    = movementGroup.get<Input, Position, Movement>(entity);
                entityUpdate.entityStates.push_back(
                    {entity, input, position, movement});
                client.inputWasDropped = false;
            }
        }

        /* Send the collected entities to this client. */
        sendUpdate(client, entityUpdate);
    }

    // Mark any dirty entities as clean.
    world.registry.clear<InputHasChanged>();
}

void ClientUpdateSystem::sendUpdate(ClientSimData& client,
                                    EntityUpdate& entityUpdate)
{
    /* If there are updates to send, send an update message. */
    if (entityUpdate.entityStates.size() > 0) {
        // Finish filling the EntityUpdate.
        entityUpdate.tickNum = sim.getCurrentTick();

        // Send the entity update message.
        network.serializeAndSend(client.netID, entityUpdate,
                                 entityUpdate.tickNum);
    }
}

} // namespace Server
} // namespace AM
