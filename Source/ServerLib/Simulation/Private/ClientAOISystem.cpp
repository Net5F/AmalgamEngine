#include "ClientAOISystem.h"
#include "Simulation.h"
#include "World.h"
#include "ComponentTypeRegistry.h"
#include "Network.h"
#include "Serialize.h"
#include "ClientSimData.h"
#include "BoundingBox.h"
#include "Cylinder.h"
#include "EntityInit.h"
#include "EntityDelete.h"
#include "ReplicatedComponentList.h"
#include "SharedConfig.h"
#include "Log.h"
#include "tracy/Tracy.hpp"
#include <algorithm>

#include "Timer.h"

namespace AM
{
namespace Server
{

ClientAOISystem::ClientAOISystem(Simulation& inSimulation, World& inWorld,
                                 ComponentTypeRegistry& inComponentTypeRegistry,
                                 Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, componentTypeRegistry{inComponentTypeRegistry}
, network{inNetwork}
, entitiesThatLeft{}
, entitiesThatEntered{}
{
}

Timer timer{};
std::size_t bytesSent{};
void ClientAOISystem::updateAOILists()
{
    ZoneScoped;

    timer.reset();
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
    if (bytesSent > 0) {
        LOG_INFO("Sent %u bytes. Took: %.9fs", bytesSent, timer.getTime());
    }
    bytesSent = 0;
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

    // Send the client an EntityInit containing each entity that entered its
    // AOI.
    EntityInit entityInit{simulation.getCurrentTick()};
    for (entt::entity entityThatEntered : entitiesThatEntered) {
        const auto& replicatedComponentList{
            registry.get<ReplicatedComponentList>(entityThatEntered)};

        // Add the entity and all of its client-relevant components to the
        // message.
        // Note: We send the entity, even if it has no client-relevant
        //       component, because there may be a build mode that cares about
        //       it.
        EntityInit::EntityData& entityData{entityInit.entityData.emplace_back(
            entityThatEntered, registry.get<Position>(entityThatEntered))};
        for (Uint8 typeIndex : replicatedComponentList.typeIndices) {
            componentTypeRegistry.storeComponent(entityThatEntered, typeIndex,
                                                 entityData.components);
        }
    }

    // Send the message.
    //network.serializeAndSend(client.netID, entityInit);
    auto message{network.serialize(entityInit)};
    bytesSent += message->size();
    network.send(client.netID, std::move(message));
}

} // namespace Server
} // namespace AM
