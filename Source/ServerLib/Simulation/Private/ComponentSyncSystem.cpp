#include "ComponentSyncSystem.h"
#include "Simulation.h"
#include "World.h"
#include "ComponentTypeRegistry.h"
#include "Network.h"
#include "SpriteData.h"
#include "ClientSimData.h"
#include "Cylinder.h"
#include "Collision.h"
#include "SharedConfig.h"
#include "Log.h"
#include "entt/entity/observer.hpp"
#include "boost/mp11/algorithm.hpp"
#include "boost/mp11/bind.hpp"
#include "tracy/Tracy.hpp"

namespace AM
{
namespace Server
{

ComponentSyncSystem::ComponentSyncSystem(
    Simulation& inSimulation, World& inWorld,
    ComponentTypeRegistry& inComponentTypeRegistry, Network& inNetwork,
    SpriteData& inSpriteData)
: simulation{inSimulation}
, world{inWorld}
, componentTypeRegistry{inComponentTypeRegistry}
, network{inNetwork}
, spriteData{inSpriteData}
{
}

void ComponentSyncSystem::sendUpdates()
{
    ZoneScoped;

    entt::registry& registry{world.registry};

    // TODO: We build a message for each updated entity, even if there aren't
    //       any clients nearby to send it to. There may be ways to optimize by
    //       making it client-by-client like MovementSyncSystem.
    // Build an EntityUpdate for each entity that has an updated component.
    auto& observedComponents{componentTypeRegistry.observedComponents};
    for (std::size_t i = 0; i < observedComponents.size(); ++i) {
        entt::observer& observer{observedComponents[i].observer};
        Uint8 typeIndex{observedComponents[i].typeIndex};

        // For each entity that was updated, push its components into its
        // message.
        for (entt::entity entity : observer) {
            componentTypeRegistry.storeComponent(
                entity, typeIndex, componentUpdateMap[entity].components);
        }

        observer.clear();
    }

    // Send each update to all nearby clients.
    auto view{registry.view<Position, ClientSimData>()};
    for (auto& [updatedEntity, componentUpdate] : componentUpdateMap) {
        // Serialize the message.
        componentUpdate.entity = updatedEntity;
        componentUpdate.tickNum = simulation.getCurrentTick();
        BinaryBufferSharedPtr message{network.serialize(componentUpdate)};

        // Get the list of entities that are in range of the updated entity.
        const std::vector<entt::entity>* entitiesInRange{nullptr};
        if (const auto* client
            = registry.try_get<ClientSimData>(updatedEntity)) {
            // Clients already have their AOI list built.
            entitiesInRange = &(client->entitiesInAOI);
        }
        else {
            const auto& updatedEntityPosition{
                view.get<Position>(updatedEntity)};
            entitiesInRange = &(world.entityLocator.getEntities(
                {updatedEntityPosition, SharedConfig::AOI_RADIUS}));
        }

        // Send the update to all nearby clients.
        for (entt::entity entity : *entitiesInRange) {
            if (view.contains(entity)) {
                const auto& client{view.get<ClientSimData>(entity)};
                network.send(client.netID, message, componentUpdate.tickNum);
            }
        }
    }

    componentUpdateMap.clear();
}

} // namespace Server
} // namespace AM
