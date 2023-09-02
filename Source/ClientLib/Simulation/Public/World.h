#pragma once

#include "WorldSignals.h"
#include "TileMap.h"

#include "entt/entity/registry.hpp"
#include "entt/signal/sigh.hpp"

struct SDL_Rect;

namespace AM
{
namespace Client
{
class SpriteData;

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
    World(SpriteData& spriteData);

    /** Signals for informing the UI of changes to the world state. */
    WorldSignals worldSignals;

    /** Entity data registry. */
    entt::registry registry;

    /** The entity that this client is controlling. */
    entt::entity playerEntity;

    /** The tile map that makes up the world. */
    TileMap tileMap;

    /**
     * Returns true if the given ID is valid and in use.
     * Note: If entt adds a storage.in_use(entity), we can replace this.
     */
    bool entityIDIsInUse(entt::entity entityID);
};

} // namespace Client
} // namespace AM
