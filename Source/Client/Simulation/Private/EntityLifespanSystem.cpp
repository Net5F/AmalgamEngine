#include "EntityLifespanSystem.h"
#include "Simulation.h"
#include "World.h"
#include "SpriteData.h"
#include "Name.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Client
{

EntityLifespanSystem::EntityLifespanSystem(Simulation& inSim,
    World& inWorld,
    SpriteData& inSpriteData,
    EventDispatcher& inNetworkEventDispatcher)
: sim{inSim}
, world{inWorld}
, spriteData{inSpriteData}
, entityInitQueue{inNetworkEventDispatcher}
, entityDeleteQueue{inNetworkEventDispatcher}
{
}

void EntityLifespanSystem::processUpdates()
{
    entt::registry& registry{world.registry};

    // Delete the entities that left our AOI on this tick (or previous ticks).
    EntityDelete* entityDelete{entityDeleteQueue.peek()};
    while ((entityDelete != nullptr)
           && (entityDelete->tickNum <= sim.getCurrentTick())) {
        if (registry.valid(entityDelete->entity)) {
            registry.destroy(entityDelete->entity);

            LOG_INFO("Entity removed: %u", entityDelete->entity);
        }
        else {
            LOG_FATAL("Asked to delete invalid entity: %u", entityDelete->entity);
        }

        entityDeleteQueue.pop();
        entityDelete = entityDeleteQueue.peek();
    }

    // Construct the entities that entered our AOI on this tick (or previous
    // ticks).
    EntityInit* entityInit{entityInitQueue.peek()};
    while ((entityInit != nullptr)
           && (entityInit->tickNum <= sim.getCurrentTick())) {
        if (!(registry.valid(entityInit->entity))) {
            // Create the entity.
            entt::entity newEntity{registry.create(entityInit->entity)};
            if (entityInit->entity != newEntity) {
                LOG_FATAL("Created entity doesn't match received entity. "
                          "Created: %u, received: %u",
                          newEntity, entityInit->entity);
            }

            // Construct their movement-related components (to be set for real
            // when we get an EntityUpdate).
            registry.emplace<Input>(entityInit->entity);
            registry.emplace<Position>(entityInit->entity);
            registry.emplace<PreviousPosition>(entityInit->entity);
            registry.emplace<Movement>(entityInit->entity);

            // Construct their name using the received name.
            registry.emplace<Name>(entityInit->entity, entityInit->name);

            // Construct their sprite using the received numericID.
            registry.emplace<Sprite>(entityInit->entity, spriteData.get(entityInit->numericID));

            // Construct a placeholder bounding box. (they'll get a real
            // bounding box once they get a position update.)
            registry.emplace<BoundingBox>(entityInit->entity, BoundingBox{});

            LOG_INFO("Entity added: %u", entityInit->entity);
        }
        else {
            LOG_FATAL("Asked to construct entity that already exists: %u", entityInit->entity);
        }

        entityInitQueue.pop();
        entityInit = entityInitQueue.peek();
    }
}

} // End namespace Client
} // End namespace AM
