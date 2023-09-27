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

    // Send the client an EntityInit containing each entity that entered its AOI.
    EntityInit entityInit{simulation.getCurrentTick()};
    for (entt::entity entityThatEntered : entitiesThatEntered) {
        const auto& replicatedComponentList{
            registry.get<ReplicatedComponentList>(entityThatEntered)};

        // Add the entity and all of its client-relevant components to the 
        // message.
        // Note: We send the entity, even if it has no client-relevant component,
        //       because there may be a build mode that cares about it.
        EntityInit::EntityData& entityData{entityInit.entityData.emplace_back(
            entityThatEntered, registry.get<Position>(entityThatEntered))};
        addComponentsToVector(registry, entityThatEntered,
                              replicatedComponentList.typeIndices,
                              entityData.components);
    }

    // Send the message.
    network.serializeAndSend(client.netID, entityInit);
}

} // namespace Server
} // namespace AM
