#include "ComponentUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "GraphicData.h"
#include "Position.h"
#include "Collision.h"
#include "ClientGraphicState.h"
#include "IsClientEntity.h"
#include "Transforms.h"
#include "Log.h"

namespace AM
{
namespace Client
{

ComponentUpdateSystem::ComponentUpdateSystem(Simulation& inSimulation,
                                             World& inWorld, Network& inNetwork,
                                             GraphicData& inGraphicData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, graphicData{inGraphicData}
, componentUpdateQueue{network.getEventDispatcher()}
{
    world.registry.on_update<GraphicState>()
        .connect<&ComponentUpdateSystem::onGraphicStateUpdated>(this);
}

ComponentUpdateSystem::~ComponentUpdateSystem()
{
    world.registry.on_update<GraphicState>()
        .disconnect<&ComponentUpdateSystem::onGraphicStateUpdated>(this);
}

void ComponentUpdateSystem::processUpdates()
{
    // We want to process updates until we've either processed the desired
    // tick, or have run out of data.
    // Note: If a component update needs to be resolved against local predicted
    //       state, that component update needs to be intercepted before it gets
    //       here. Otherwise, it will be auto-applied over your predicted state.
    Uint32 desiredTick{simulation.getReplicationTick()};

    // Immediately process any player entity messages, push the rest into a
    // secondary queue.
    {
        ComponentUpdate componentUpdate{};
        while (componentUpdateQueue.pop(componentUpdate)) {
            if (componentUpdate.entity == world.playerEntity) {
                processComponentUpdate(componentUpdate);
            }
            else {
                componentUpdateSecondaryQueue.push(componentUpdate);
            }
        }
    }

    // Process all messages in the secondary queue.
    while (!(componentUpdateSecondaryQueue.empty())) {
        ComponentUpdate& componentUpdate{componentUpdateSecondaryQueue.front()};

        // If we've reached the desired tick, save the rest of the messages for
        // later.
        if (componentUpdate.tickNum > desiredTick) {
            break;
        }

        // Process the message.
        processComponentUpdate(componentUpdate);

        componentUpdateSecondaryQueue.pop();
    }
}

void ComponentUpdateSystem::processComponentUpdate(
    const ComponentUpdate& componentUpdate)
{
    entt::registry& registry{world.registry};

    // Construct or update any updated components.
    for (const auto& componentVariant : componentUpdate.updatedComponents) {
        std::visit(
            [&](const auto& component) {
                using T = std::decay_t<decltype(component)>;
                registry.emplace_or_replace<T>(componentUpdate.entity,
                                               component);
            },
            componentVariant);
    }

    // Destroy any destroyed components.
    for (Uint8 componentIndex : componentUpdate.destroyedComponents) {
        boost::mp11::mp_with_index<
            boost::mp11::mp_size<ReplicatedComponentTypes>>(
            componentIndex, [&](auto I) {
                using ComponentType
                    = boost::mp11::mp_at_c<ReplicatedComponentTypes, I>;
                registry.remove<ComponentType>(componentUpdate.entity);
            });
    }
}

void ComponentUpdateSystem::onGraphicStateUpdated(entt::registry& registry,
                                                  entt::entity entity)
{
    // Since the graphic state was updated, we need to update the entity's
    // collision.
    // Note: Entity collision always comes from its IdleSouth graphic.
    auto [position, graphicState, clientGraphicState]
        = registry.get<Position, GraphicState, ClientGraphicState>(entity);
    const EntityGraphicSet& graphicSet{
        graphicData.getEntityGraphicSet(graphicState.graphicSetID)};

    // Note: We assume that an entity with GraphicState always has a
    //       Collision.
    const BoundingBox& modelBounds{graphicSet.getCollisionModelBounds()};
    const Collision& collision{
        registry.patch<Collision>(entity, [&](Collision& collision) {
            collision.modelBounds = modelBounds;
            collision.worldBounds
                = Transforms::modelToWorldEntity(modelBounds, position);
        })};

    // Update their collision in the locator.
    CollisionObjectType::Value objectType{
        world.registry.all_of<IsClientEntity>(entity)
            ? CollisionObjectType::ClientEntity
            : CollisionObjectType::NonClientEntity};
    world.collisionLocator.updateEntity(entity, collision.worldBounds,
                                        objectType);

    // Default the entity's current graphic type to IdleSouth since it 
    // always must be valid and we don't know if the new graphic set has 
    // the old type.
    // GraphicSystem will set it to a real value the next time it runs.
    clientGraphicState.graphicType = EntityGraphicType::IdleSouth;
}

} // namespace Client
} // namespace AM
