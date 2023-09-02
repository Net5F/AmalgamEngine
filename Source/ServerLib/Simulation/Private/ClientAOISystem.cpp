#include "ClientAOISystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Serialize.h"
#include "ClientSimData.h"
#include "BoundingBox.h"
#include "EntityType.h"
#include "Name.h"
#include "Sprite.h"
#include "SpriteSets.h"
#include "Interactions.h"
#include "EntityDelete.h"
#include "ClientEntityInit.h"
#include "DynamicObjectInit.h"
#include "Cylinder.h"
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
    for (auto [entity, client, position] : view.each()) {
        // Clear our lists.
        entitiesThatLeft.clear();
        entitiesThatEntered.clear();

        // Get the list of entities that are in this entity's AOI.
        std::vector<entt::entity>& currentAOIEntities{
            world.entityLocator.getEntities(
                {position, SharedConfig::AOI_RADIUS})};

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
    auto view{world.registry.view<EntityType, Position>()};

    // Send the client an EntityInit for each entity that entered its AOI.
    for (entt::entity entityThatEntered : entitiesThatEntered) {
        auto [entityType, position]
            = view.get<EntityType, Position>(entityThatEntered);

        // Send the appropriate init message for the entity.
        if (entityType == EntityType::ClientEntity) {
            const auto& name{world.registry.get<Name>(entityThatEntered)};
            const auto& sprite{world.registry.get<Sprite>(entityThatEntered)};
            const auto& rotation{world.registry.get<Rotation>(entityThatEntered)};
            network.serializeAndSend(
                client.netID,
                ClientEntityInit{simulation.getCurrentTick(), entityThatEntered,
                                 name.name, position, rotation,
                                 static_cast<Uint8>(sprite.numericID)});
        }
        else if (entityType == EntityType::DynamicObject) {
            const auto& name{world.registry.get<Name>(entityThatEntered)};
            const auto& spriteSet{
                world.registry.get<ObjectSpriteSet>(entityThatEntered)};
            const auto& rotation{world.registry.get<Rotation>(entityThatEntered)};
            const auto& interactions{
                world.registry.get<Interactions>(entityThatEntered)};
            network.serializeAndSend(
                client.netID,
                DynamicObjectInit{simulation.getCurrentTick(),
                                  entityThatEntered, name.name, position,
                                  rotation, spriteSet.numericID, interactions});
        }
    }
}

} // namespace Server
} // namespace AM
