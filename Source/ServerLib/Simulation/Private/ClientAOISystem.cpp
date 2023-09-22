#include "ClientAOISystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Serialize.h"
#include "ClientSimData.h"
#include "BoundingBox.h"
#include "Cylinder.h"
#include "ReplicatedComponent.h"
#include "EntityInit.h"
#include "EntityDelete.h"
#include "ReplicatedComponentList.h"
#include "SharedConfig.h"
#include "Log.h"
#include "tracy/Tracy.hpp"
#include "boost/mp11/list.hpp"
#include "boost/mp11/algorithm.hpp"
#include <algorithm>

namespace AM
{
namespace Server
{

/**
 * Retrieves the types in componentIndices for the given entity and pushes 
 * them into componentVec.
 *
 * Note: This is a free function to reduce includes in the header.
 */
void addComponentsToVector(entt::registry& registry, entt::entity entity,
                           const std::vector<Uint8>& componentIndices,
                           std::vector<ReplicatedComponent>& componentVec)
{
    for (Uint8 componentIndex : componentIndices) {
        boost::mp11::mp_with_index<
            boost::mp11::mp_size<ReplicatedComponentTypes>>(
            componentIndex, [&](auto I) {
                using T = boost::mp11::mp_at_c<ReplicatedComponentTypes, I>;
                if constexpr (std::is_empty_v<T>) {
                    // Note: Can't registry.get() empty types.
                    componentVec.push_back(T{});
                }
                else {
                    componentVec.push_back(registry.get<T>(entity));
                }
            });
    }
}

ClientAOISystem::ClientAOISystem(Simulation& inSimulation, World& inWorld,
                                 Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, entitiesThatLeft{}
, entitiesThatEntered{}
, entityInitMap{}
{
}

void ClientAOISystem::updateAOILists()
{
    ZoneScoped;

    // Update every client entity's AOI list.
    auto view{world.registry.view<ClientSimData, Position>()};
    for (auto [entity, client, position] : view.each()) {
        // Clear our lists.
        entitiesThatLeft.clear();
        entitiesThatEntered.clear();

        // Get the list of entities that are in this entity's AOI.
        std::vector<entt::entity>& currentAOIEntities{
            world.entityLocator.getEntities(
                Cylinder{position, SharedConfig::AOI_RADIUS})};

        // Sort the list.
        std::sort(currentAOIEntities.begin(), currentAOIEntities.end());

        // Fill entitiesThatLeft with the entities that left this entity's AOI.
        std::vector<entt::entity>& oldAOIEntities{client.entitiesInAOI};
        std::set_difference(oldAOIEntities.begin(), oldAOIEntities.end(),
                            currentAOIEntities.begin(),
                            currentAOIEntities.end(),
                            std::back_inserter(entitiesThatLeft));

        // Process the entities that left this entity's AOI.
        if (entitiesThatLeft.size() > 0) {
            processEntitiesThatLeft(client);
        }

        // Fill entitiesThatEntered with the entities that entered this entity's
        // AOI.
        std::set_difference(currentAOIEntities.begin(),
                            currentAOIEntities.end(), oldAOIEntities.begin(),
                            oldAOIEntities.end(),
                            std::back_inserter(entitiesThatEntered));

        // Process the entities that entered this entity's AOI.
        if (entitiesThatEntered.size() > 0) {
            processEntitiesThatEntered(client);
        }

        // Save the new list.
        client.entitiesInAOI = currentAOIEntities;
    }

    // Note: If it's ever worthwhile, we could wait until the entity actually 
    //       changes to clear its message from the map.
    // Clear the message map, since they won't be valid next tick.
    entityInitMap.clear();
}

void ClientAOISystem::processEntitiesThatLeft(ClientSimData& client)
{
    // Send the client an EntityDelete for each entity that left its AOI.
    for (entt::entity entityThatLeft : entitiesThatLeft) {
        network.serializeAndSend(
            client.netID,
            EntityDelete{simulation.getCurrentTick(), entityThatLeft});
    }
}

void ClientAOISystem::processEntitiesThatEntered(ClientSimData& client)
{
    entt::registry& registry{world.registry};

    // Send the client an EntityInit for each entity that entered its AOI.
    for (entt::entity entityThatEntered : entitiesThatEntered) {
        // If we already built a message for this entity, send it.
        if (auto pair = entityInitMap.find(entityThatEntered);
            pair != entityInitMap.end()) {
            network.send(client.netID, pair->second);
            continue;
        }

        const auto& replicatedComponentList{
            registry.get<ReplicatedComponentList>(entityThatEntered)};

        // Build a message with all of the entity's client-relevant components.
        // Note: We send the entity, even if it has no client-relevant component,
        //       because there may be a build mode that cares about it.
        EntityInit entityInit{simulation.getCurrentTick(), entityThatEntered};
        addComponentsToVector(registry, entityThatEntered,
                              replicatedComponentList.typeIndices,
                              entityInit.components);

        // Serialize the message and save it in the map, in case any other 
        // clients need to be sent the same data.
        BinaryBufferSharedPtr message{network.serialize(entityInit)};
        entityInitMap[entityThatEntered] = message;

        // Send the message.
        network.send(client.netID, message);
    }
}

} // namespace Server
} // namespace AM
