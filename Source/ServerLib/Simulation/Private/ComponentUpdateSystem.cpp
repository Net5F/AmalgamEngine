#include "ComponentUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "EngineComponentLists.h"
#include "ProjectComponentLists.h"
#include "ReplicatedComponent.h"
#include "ClientSimData.h"
#include "Cylinder.h"
#include "Collision.h"
#include "ISimulationExtension.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include "Log.h"
#include "boost/mp11/algorithm.hpp"
#include "boost/mp11/map.hpp"
#include "boost/mp11/bind.hpp"
#include "tracy/Tracy.hpp"

namespace AM
{
namespace Server
{

//-----------------------------------------------------------------------------
// Templated type setup
//-----------------------------------------------------------------------------
/**
 * See comment in EngineComponentLists.h
 */
using ObservedComponentMap
    = boost::mp11::mp_append<EngineComponentLists::ObservedComponentMap,
                             ProjectComponentLists::ObservedComponentMap>;
using ObservedComponents = boost::mp11::mp_map_keys<ObservedComponentMap>;
static_assert(boost::mp11::mp_is_map<ObservedComponentMap>::value,
              "One of the ObservedComponentMaps has invalid syntax.");
static_assert(
    boost::mp11::mp_all_of_q<
        boost::mp11::mp_second<ObservedComponentMap>,
        boost::mp11::mp_bind_front<boost::mp11::mp_contains,
                                   ReplicatedComponentTypes>>::value,
    "One of the SendComponents was not found in ReplicatedComponentTypes.");

/** A group and update observer for each observed component type. */
std::array<entt::observer, boost::mp11::mp_size<ObservedComponents>::value>
    observers;

//-----------------------------------------------------------------------------
// ComponentUpdateSystem members
//-----------------------------------------------------------------------------
ComponentUpdateSystem::ComponentUpdateSystem(Simulation& inSimulation,
                                             World& inWorld, Network& inNetwork,
                                             SpriteData& inSpriteData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, extension{nullptr}
, componentUpdateRequestQueue{inNetwork.getEventDispatcher()}
{
    boost::mp11::mp_for_each<ObservedComponents>([&](auto I) {
        using ObservedComponent = decltype(I);
        constexpr std::size_t index{
            boost::mp11::mp_find<ObservedComponents, ObservedComponent>::value};

        // Note: If a client is near an entity when it's constructed, it'll 
        //       receive both an EntityInit and an EntityUpdate (from the group
        //       observer). It'd be nice if we could find a way to just send one,
        //       but until then it isn't a huge cost.
        observers[index].connect(world.registry,
                                 entt::collector.group<ObservedComponent>()
                                     .template update<ObservedComponent>());
    });

    world.registry.on_update<AnimationState>()
        .connect<&ComponentUpdateSystem::onAnimationStateUpdated>(this);
}

void ComponentUpdateSystem::processUpdateRequests()
{
    ZoneScoped;

    entt::registry& registry{world.registry};

    // Process any waiting update requests.
    ComponentUpdateRequest componentUpdateRequest{};
    while (componentUpdateRequestQueue.pop(componentUpdateRequest)) {
        // If the project says the request isn't valid, skip it.
        if ((extension != nullptr)
            && !(extension->isComponentUpdateRequestValid(componentUpdateRequest))) {
            continue;
        }

        // Update or add the given components.
        for (const auto& componentVariant : componentUpdateRequest.components) {
            std::visit(
                [&](const auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    registry.emplace_or_replace<T>(
                        componentUpdateRequest.entity, component);
                },
                componentVariant);
        }
    }
}

void ComponentUpdateSystem::sendUpdates()
{
    ZoneScoped;

    entt::registry& registry{world.registry};

    // Note: We build a message for each updated entity, even if there aren't 
    //       any clients nearby to send it to. There may be ways to optimize.
    // Build an EntityUpdate for each entity that has an updated component.
    boost::mp11::mp_for_each<ObservedComponents>([&](auto I) {
        using ObservedComponent = decltype(I);
        using SendComponents = boost::mp11::mp_pop_front<
            boost::mp11::mp_map_find<ObservedComponentMap, ObservedComponent>>;
        constexpr std::size_t index{
            boost::mp11::mp_find<ObservedComponents, ObservedComponent>::value};

        // For each entity that was updated, push its components into its 
        // message.
        for (entt::entity entity : observers[index]) {
            boost::mp11::mp_for_each<SendComponents>([&](auto I) {
                using T = decltype(I);
                if constexpr (std::is_empty_v<T>) {
                    // Note: Can't registry.get() empty types.
                    componentUpdateMap[entity].components.push_back(T{});
                }
                else {
                    const auto& component{registry.get<T>(entity)};
                    componentUpdateMap[entity].components.push_back(component);
                }
            });
        }

        observers[index].clear();
    });

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
            const auto& updatedEntityPosition{view.get<Position>(updatedEntity)};
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

void ComponentUpdateSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
}

void ComponentUpdateSystem::onAnimationStateUpdated(entt::registry& registry,
                                                    entt::entity entity)
{
    const auto& animationState{registry.get<AnimationState>(entity)};
    auto [position, sprite] = registry.try_get<Position, Sprite>(entity);
    if (position && sprite) {
        const Sprite* newSprite{
            spriteData.getObjectSpriteSet(animationState.spriteSetID)
                .sprites[animationState.spriteIndex]};

        // Note: We assume that an entity with Position and AnimationState 
        //       always has a Collision.
        Collision& collision{
            registry.patch<Collision>(entity, [&](Collision& collision) {
                collision.modelBounds = newSprite->modelBounds;
                collision.worldBounds = Transforms::modelToWorldCentered(
                    newSprite->modelBounds, *position);
            })};

        world.entityLocator.setEntityLocation(entity, collision.worldBounds);
    }
}

} // namespace Server
} // namespace AM
