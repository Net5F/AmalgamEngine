#pragma once

#include "CollisionLayerType.h"
#include "entt/fwd.hpp"
#include "bitsery/bitsery.h"

namespace AM
{

/**
 * Tracks how an entity should be configured in CollisionLocator.
 */
struct CollisionBitSets {
public:
    /** Sets to defaults. */
    CollisionBitSets();

    /** Sets to defaults, accounting for entity state. */
    CollisionBitSets(entt::entity entity, entt::registry& registry);

    /**
     * Sets the layers that this entity will appear in.
     */
    void setCollisionLayers(CollisionLayerBitSet inCollisionLayers,
                            entt::entity entity, entt::registry& registry);

    /**
     * Sets the layers that this entity will scan for collisions during movement.
     */
    void setCollisionMask(CollisionLayerBitSet inCollisionMask);

    CollisionLayerBitSet getCollisionLayers() const;

    CollisionLayerBitSet getCollisionMask() const;

private:
    /**
     * By default, entities should block collision but not block LoS.
     */
    static constexpr CollisionLayerBitSet DEFAULT_COLLISION_LAYERS{
        CollisionLayerType::BlockCollision};

    /**
     * By default, entities should collide with terrain and wall tile layers.
     */
    static constexpr CollisionLayerBitSet DEFAULT_COLLISION_MASK{
        CollisionLayerType::TerrainWall};

    /** The layers that this entity appears in. */
    CollisionLayerBitSet collisionLayers;

    /** The layers that this entity will scan for collisions during movement. */
    CollisionLayerBitSet collisionMask;

    friend class bitsery::Access;
    template<typename S>
    void serialize(S& serializer)
    {
        serializer.value2b(collisionLayers);
        serializer.value2b(collisionMask);
    }
};

} // End namespace AM
