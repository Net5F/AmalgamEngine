#pragma once

#include "TileMap.h"
#include "ScreenPoint.h"

#include "entt/entity/registry.hpp"

class SDL_Rect;

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

    /** Entity data registry. */
    entt::registry registry;

    /** We save the player entity ID since it's more convenient than searching
        for the PlayerState component and getting the entity from that every
        time we need it. */
    entt::entity playerEntity;

    /** The tile map that makes up the world. */
    TileMap tileMap;

    /** The mouse's current position in screen space.
        Temporarily here, should be removed eventually. */
    ScreenPoint mouseScreenPoint;
};

} // namespace Client
} // namespace AM
