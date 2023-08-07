#include "ClientAOISystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Serialize.h"
#include "ClientSimData.h"
#include "BoundingBox.h"
#include "Name.h"
#include "Sprite.h"
#include "EntityDelete.h"
#include "ClientEntityInit.h"
#include "NonClientEntityInit.h"
#include "SharedConfig.h"
#include "Log.h"
#include "Tracy.hpp"
#include <algorithm>

namespace AM
{
namespace Server
{
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
    for (entt::entity entity : view) {
        auto [client, position] = view.get<ClientSimData, Position>(entity);

        // Clear our lists.
        entitiesThatLeft.clear();
        entitiesThatEntered.clear();

        // Get the list of entities that are in this entity's AOI.
        std::vector<entt::entity>& currentAOIEntities{
            world.entityLocator.getEntitiesFine(position,
                                                SharedConfig::AOI_RADIUS)};

        // Remove this entity from the list, if it's in there.
        // (We don't want to add it to its own list.)
        auto entityIt{std::find(currentAOIEntities.begin(),
                                currentAOIEntities.end(), entity)};
        if (entityIt != currentAOIEntities.end()) {
            currentAOIEntities.erase(entityIt);
        }

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
    auto view{world.registry.view<Position, Sprite, Name>()};

    // Send the client an EntityInit for each entity that entered its AOI.
    for (entt::entity entityThatEntered : entitiesThatEntered) {
        auto [position, sprite] = view.get<Position, Sprite>(entityThatEntered);

        // If it's a client entity, send a ClientEntityInit.
        if (world.registry.all_of<ClientSimData>(entityThatEntered)) {
            Name& name{view.get<Name>(entityThatEntered)};
            network.serializeAndSend(
                client.netID,
                ClientEntityInit{simulation.getCurrentTick(), entityThatEntered,
                                 name.name, position, sprite.numericID});
        }
        else {
            // Non-client entity, send a NonClientEntityInit.
            network.serializeAndSend(
                client.netID, NonClientEntityInit{simulation.getCurrentTick(),
                                                  entityThatEntered, position,
                                                  sprite.numericID});
        }
    }
}

} // namespace Server
} // namespace AM
