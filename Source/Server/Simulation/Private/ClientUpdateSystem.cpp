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
#include "IsDirty.h"
#include "Ignore.h"
#include "Log.h"
#include <algorithm>
#include "Profiler.h"

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
    // Init the groups that we'll be using.
    auto clientGroup = world.registry.group<ClientSimData>(entt::get<Position>);
    ignore(clientGroup);
    auto movementGroup = world.registry.group<Input, Position, Movement>();
    ignore(movementGroup);
}

void ClientUpdateSystem::sendClientUpdates()
{
    SCOPED_CPU_SAMPLE(sendClientUpdate);

    // Collect the dirty entities.
    auto dirtyView = world.registry.view<IsDirty>();
    auto movementGroup = world.registry.group<Input, Position, Movement>();
    std::vector<EntityStateRefs> dirtyEntities;
    for (entt::entity entity : dirtyView) {
        auto [input, position, movement]
            = movementGroup.get<Input, Position, Movement>(entity);
        dirtyEntities.push_back({entity, input, position, movement});
    }

    /* Update clients as necessary. */
    auto clientGroup = world.registry.group<ClientSimData>(entt::get<Position>);
    for (entt::entity entity : clientGroup) {
        // Center this client's AoI on its current position.
        auto [client, clientPosition]
            = clientGroup.get<ClientSimData, Position>(entity);

        /* Collect the entities that need to be sent to this client. */
        EntityUpdate entityUpdate{};

        // Add all the dirty entities that are in range.
        for (EntityStateRefs& state : dirtyEntities) {
            // Calc the distance from this dirty entity to the client entity.
            // Note: This uses the top left of the entity's position instead of
            //       the center since it's easy and seems fine. We can change
            //       to the center if we need to.
            Position diff{clientPosition - state.position};
            double distanceSquared { std::abs(
                (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z)) };

            // If the dirty entity is in the client entity's AOI, push it.
            if (distanceSquared <= SharedConfig::AOI_RADIUS_SQUARED) {
                entityUpdate.entityStates.push_back({state.entity, state.input,
                                                     state.position,
                                                     state.movement});
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
    world.registry.clear<IsDirty>();
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
