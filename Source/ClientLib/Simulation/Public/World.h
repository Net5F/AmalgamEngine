#pragma once

#include "TileMap.h"
#include "EntityLocator.h"
#include "CollisionLocator.h"
#include "CastHelper.h"
#include "AVEntityID.h"
#include "entt/entity/registry.hpp"

struct SDL_Rect;

namespace AM
{
class CastableData;

namespace Client
{
struct SimulationContext;
class Simulation;
class Network;
class GraphicData;
class ItemData;

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
    World(const SimulationContext& inSimContext);

    /** Server-synchronized entity data registry. For entity state that we try
        to keep in sync with the server. */
    entt::registry registry;

    /** Audio/visual entity data registry. A/V entities are processed 
        by AVSystem and aren't synchronized with the server and other 
        clients. */
    entt::basic_registry<AVEntityID> avRegistry;

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

    /** Helper class for casting Castables. */
    CastHelper castHelper;
};

} // namespace Client
} // namespace AM
