#pragma once

#include "SimDefs.h"
#include "Sprite.h"
#include "ScreenPoint.h"

#include "entt/entity/registry.hpp"

#include <vector>

namespace AM
{
namespace Client
{
/**
 * Holds world state and manages the persistence of that state.
 *
 * Also provides helpers for common uses of world state.
 */
class World
{
public:
    // TODO: Replace inSpriteTexturePtr with a texture loader.
    World(const std::shared_ptr<SDL2pp::Texture>& inSpriteTexturePtr);

    /** Entity data registry. */
    entt::registry registry;

    /** We save the player entity ID since it's more convenient than searching
        for the PlayerState component and getting the entity from that every
        time we need it. */
    entt::entity playerEntity;

    /** The map of tiles that makes up our world's terrain. */
    std::vector<Sprite> terrainMap;

    /** The mouse's current position in screen space. */
    ScreenPoint mouseScreenPoint;
};

} // namespace Client
} // namespace AM
