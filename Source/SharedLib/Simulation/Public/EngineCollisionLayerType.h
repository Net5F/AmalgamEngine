#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** A type that can be used for collision layer bitsets.
    Must always match the underlying type of EngineCollisionLayerType::Value. */
using CollisionLayerBitSet = Uint16;

/**
 * The collision layer types that the engine provides.
 *
 * Note: Don't use this enum directly, use CollisionLayerType (it combines the 
 *       engine's and the project's collision layer types)
 */
struct EngineCollisionLayerType
{
    enum Value : Uint16 {
        /** Terrain and Wall tile layers. */
        TerrainWall = 1 << 0,
        /** Object tile layers. */
        Object = 1 << 1,
        ClientEntity = 1 << 2,
        NonClientEntity = 1 << 3,
        /** Entities that can be collided with. */
        BlockCollision = 1 << 4,
        /** Entities that block Line-of-Sight raycasts. */
        BlockLoS = 1 << 5,
    };
};

} // End namespace AM
