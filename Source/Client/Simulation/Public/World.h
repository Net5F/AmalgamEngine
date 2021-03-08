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

class TileIndex;

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

    using TileLayer = std::vector<Sprite>;
    /** The layers of our tile map.
        Layer 0 is always the base terrain, the rest will sequentially be drawn
        on top. */
    std::vector<TileLayer> mapLayers;

    /** The mouse's current position in screen space. */
    ScreenPoint mouseScreenPoint;

private:
    void addTile(unsigned int layer, const TileIndex& index, const Sprite& sprite);
};

} // namespace Client
} // namespace AM
