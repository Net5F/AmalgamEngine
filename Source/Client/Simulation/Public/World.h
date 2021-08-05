#pragma once

#include "Sprite.h"
#include "ScreenPoint.h"

#include "entt/entity/registry.hpp"

#include <vector>

class SDL_Rect;

namespace AM
{
class AssetCache;
class BoundingBox;
class TileIndex;

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
    World(AssetCache& inAssetCache);

    /** Entity data registry. */
    entt::registry registry;

    /** We save the player entity ID since it's more convenient than searching
        for the PlayerState component and getting the entity from that every
        time we need it. */
    entt::entity playerEntity;

    // TODO: Turn the map into its own data type.
    using TileLayer = std::vector<Sprite>;
    /** The layers of our tile map.
        Layer 0 is always the base terrain, the rest will sequentially be drawn
        on top. */
    std::vector<TileLayer> mapLayers;

    /** The mouse's current position in screen space. */
    ScreenPoint mouseScreenPoint;

private:
    /**
     * Adds a tile sprite to the tile map at the given layer and position.
     */
    void addTile(unsigned int layer, const TileIndex& index,
                 TextureHandle texture, const SDL_Rect& extent,
                 const BoundingBox& modelBounds);

    /** Used for loading map textures. */
    AssetCache& assetCache;
};

} // namespace Client
} // namespace AM
