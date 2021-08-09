#include "World.h"
#include "AssetCache.h"
#include "TileIndex.h"
#include "BoundingBox.h"
#include "Position.h"
#include "Paths.h"
#include "SharedConfig.h"
#include "Log.h"

#include <SDL2/SDL_rect.h>

namespace AM
{
namespace Client
{
World::World(AssetCache& inAssetCache)
: playerEntity(entt::null)
, mapLayers(3)
, mouseScreenPoint{}
, assetCache{inAssetCache}
{
    // TODO: Load textures from a file, through a class that Application owns.

    // Init our layers.
    for (TileLayer& layer : mapLayers) {
        layer.resize(SharedConfig::WORLD_WIDTH * SharedConfig::WORLD_HEIGHT);
    }

    // Fill layer 0 (terrain layer).
    // TODO: Load this from some storage format.
    SDL_Rect floorPosInTexture{(256 * 6), 0, 256, 512};
    TextureHandle texture
        = assetCache.loadTexture(Paths::TEXTURE_DIR + "iso_test_sprites.png");
    std::fill(mapLayers[0].begin(), mapLayers[0].end(),
              Sprite{texture, floorPosInTexture, 256, 512});

    // Add some rugs to layer 1.
    SDL_Rect rugPosInTexture{(256 * 6), (512 * 1), 256, 512};
    addTile(1, {0, 3}, texture, rugPosInTexture, BoundingBox{});
    addTile(1, {4, 3}, texture, rugPosInTexture, BoundingBox{});
    addTile(1, {3, 6}, texture, rugPosInTexture, BoundingBox{});
    addTile(1, {9, 2}, texture, rugPosInTexture, BoundingBox{});
    addTile(1, {1, 5}, texture, rugPosInTexture, BoundingBox{});

    // Add some walls to layer 2.
    SDL_Rect wallPosInTexture{(256 * 8), (512 * 1), 256, 512};
    addTile(2, {2, 0}, texture, wallPosInTexture,
            BoundingBox{0, 13.5, 0, 32, 0, 73});
    addTile(2, {2, 1}, texture, wallPosInTexture,
            BoundingBox{0, 13.5, 0, 32, 0, 73});
    addTile(2, {2, 2}, texture, wallPosInTexture,
            BoundingBox{0, 13.5, 0, 32, 0, 73});

    SDL_Rect wallPosInTexture2{(256 * 8), (512 * 2), 256, 512};
    addTile(2, {0, 2}, texture, wallPosInTexture2,
            BoundingBox{0, 32, 0, 13.5, 0, 73});
}

void World::addTile(unsigned int layer, const TileIndex& index,
                    TextureHandle texture, const SDL_Rect& extent,
                    const BoundingBox& modelBounds)
{
    // Convert the 2d tile position into an index into the 1d array.
    unsigned int linearizedIndex
        = index.y * SharedConfig::WORLD_WIDTH + index.x;
    Sprite& sprite = mapLayers[layer][linearizedIndex];

    // Build the sprite.
    sprite.texture = texture;
    sprite.textureExtent = extent;
    sprite.modelBounds = modelBounds;
}

} // namespace Client
} // namespace AM
