#pragma once

#include "EngineCollisionLayerType.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * The collision layer types that are available to the collision system.
 * 
 * Used by CollisionLocator for collision and raycasting.
 */
struct CollisionLayerType
{
    enum Value : Uint16 {
        // Engine collision layer types (copied here so we can use one strongly-
        // typed enum).
        TerrainWall = static_cast<Uint8>(EngineCollisionLayerType::TerrainWall),
        Object = static_cast<Uint8>(EngineCollisionLayerType::Object),
        ClientEntity = static_cast<Uint8>(EngineCollisionLayerType::ClientEntity),
        NonClientEntity
        = static_cast<Uint8>(EngineCollisionLayerType::NonClientEntity),
        BlockCollision = static_cast<Uint8>(EngineCollisionLayerType::BlockCollision),
        BlockLoS = static_cast<Uint8>(EngineCollisionLayerType::BlockLoS),

        // Project collision layer types.
        // Note: To avoid conflicting with future layer types that the engine 
        //       may add, we start at the highest bit and move downwards.
        // MyLayerType1 = 1 << (15 - 0),
        // MyLayerType2 = 1 << (15 - 1),
    };
};

} // End namespace AM
