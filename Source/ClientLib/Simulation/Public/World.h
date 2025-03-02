#pragma once

#include "ItemData.h"
#include "TileMap.h"
#include "EntityLocator.h"
#include "CollisionLocator.h"
#include "entt/entity/registry.hpp"

struct SDL_Rect;

namespace AM
{
namespace Client
{
class GraphicData;

/**
 * Owns and manages the persistence of all world state.
 *
 * The client's world state consists of:
 *   Map data
 *     See TileMap.h
 *   Entity data
 *     Maintained at runtime in an ECS registry.
 *
 * Also provides helpers for common uses of world state.
 */
class World
{
public:
    World(GraphicData& graphicData);

    /** Entity data registry. */
    entt::registry registry;

    /** Item data templates. */
    ItemData itemData;

    /** The entity that this client is controlling. */
    entt::entity playerEntity;

    /** Spatial partitioning grid for efficiently locating entities by
        position. */
    EntityLocator entityLocator;

    /** Spatial partitioning grid for efficiently locating entities and tile 
        layers by their collision volumes. */
    CollisionLocator collisionLocator;

    /** The tile map that makes up the world. */
    TileMap tileMap;
};

} // namespace Client
} // namespace AM
