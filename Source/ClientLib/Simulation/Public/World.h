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

    /** We save the player entity ID since it's more convenient than searching
        for the PlayerState component and getting the entity from that every
        time we need it. */
    entt::entity playerEntity;

    /** The tile map that makes up the world. */
    TileMap tileMap;
};

} // namespace Client
} // namespace AM
