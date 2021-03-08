#include "World.h"
#include "TileIndex.h"
#include "Log.h"

namespace AM
{
namespace Client
{
World::World(const std::shared_ptr<SDL2pp::Texture>& inSpriteTexturePtr)
: playerEntity(entt::null)
, mapLayers(3)
, mouseScreenPoint{}
{
    // Init our layers.
    for (TileLayer& layer : mapLayers) {
        layer.resize(WORLD_WIDTH * WORLD_HEIGHT);
    }

    // Fill layer 0 (terrain layer).
    // TODO: Load this from some storage format.
    SDL2pp::Rect floorPosInTexture{(256 * 6), 0, 256, 512};
    std::fill(mapLayers[0].begin(), mapLayers[0].end(),
              Sprite{inSpriteTexturePtr, floorPosInTexture, 256, 512});

    // Add some rugs to layer 1.
    SDL2pp::Rect rugPosInTexture{(256 * 6), (512 * 1), 256, 512};
    Sprite rugSprite{inSpriteTexturePtr, rugPosInTexture, 256, 512};
    addTile(1, {0, 3}, rugSprite);
    addTile(1, {4, 3}, rugSprite);
    addTile(1, {3, 6}, rugSprite);
    addTile(1, {9, 2}, rugSprite);
    addTile(1, {1, 5}, rugSprite);

    // Add some walls to layer 2.
    SDL2pp::Rect wallPosInTexture{(256 * 8), (512 * 1), 256, 512};
    Sprite wallSprite{inSpriteTexturePtr, wallPosInTexture, 256, 512};
    addTile(2, {2, 0}, wallSprite);
    addTile(2, {2, 1}, wallSprite);
    addTile(2, {2, 2}, wallSprite);

    SDL2pp::Rect wallPosInTexture2{(256 * 8), (512 * 2), 256, 512};
    Sprite wallSprite2{inSpriteTexturePtr, wallPosInTexture2, 256, 512};
    addTile(2, {0, 2}, wallSprite2);
}

void World::addTile(unsigned int layer, const TileIndex& index, const Sprite& sprite)
{
    // Convert the 2d tile position into an index into the 1d array.
    unsigned int linearizedIndex = index.y * WORLD_WIDTH + index.x;
    mapLayers[layer][linearizedIndex] = sprite;
}

} // namespace Client
} // namespace AM
