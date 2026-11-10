#include "CollisionBitSets.h"
#include "Input.h"
#include "IsPlayerEntity.h"
#include "entt/entity/registry.hpp"

namespace AM
{

CollisionBitSets::CollisionBitSets()
: collisionLayers{DEFAULT_COLLISION_LAYERS}
, collisionMask{DEFAULT_COLLISION_MASK}
{
}

CollisionBitSets::CollisionBitSets(entt::entity entity,
                                   entt::registry& registry)
{
    setCollisionLayers(DEFAULT_COLLISION_LAYERS, entity, registry);
    setCollisionMask(DEFAULT_COLLISION_MASK);
}

void CollisionBitSets::setCollisionLayers(
    CollisionLayerBitSet inCollisionLayers, entt::entity entity,
    entt::registry& registry)
{
    collisionLayers = inCollisionLayers;

    // Make sure none of the tile layer types are set.
    collisionLayers
        &= ~(CollisionLayerType::TerrainWall | CollisionLayerType::Object);

    // Set PlayerEntity/NonPlayerEntity ourselves, so it's always accurate.
    collisionLayers &= ~(CollisionLayerType::PlayerEntity
                         | CollisionLayerType::NonPlayerEntity);
    if (registry.all_of<IsPlayerEntity>(entity)) {
        collisionLayers |= CollisionLayerType::PlayerEntity;
    }
    else {
        collisionLayers |= CollisionLayerType::NonPlayerEntity;
    }

    // If the entity is movement-enabled, don't let it block collision.
    // Note: See EntityMover.h for more info.
    if (registry.all_of<Input>(entity)) {
        collisionLayers &= ~CollisionLayerType::BlockCollision;
    }
}

void CollisionBitSets::setCollisionMask(CollisionLayerBitSet inCollisionMask)
{
    // Note: All entities always collide with TerrainWall. We do this to
    //       avoid devs accidentally removing TerrainWall collision and
    //       having their non-player entity fall through the floor.
    collisionMask = inCollisionMask | CollisionLayerType::TerrainWall;
}

CollisionLayerBitSet CollisionBitSets::getCollisionLayers() const
{
    return collisionLayers;
}

CollisionLayerBitSet CollisionBitSets::getCollisionMask() const
{
    return collisionMask;
}

} // End namespace AM
