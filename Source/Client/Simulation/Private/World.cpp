#include "World.h"
#include "Log.h"

namespace AM
{
namespace Client
{
World::World(const std::shared_ptr<SDL2pp::Texture>& inSpriteTexturePtr)
: playerEntity(entt::null)
, mapLayers(2)
, mouseScreenPoint{}
{
    // Init our layers.
    for (TileLayer& layer : mapLayers) {
        layer.resize(WORLD_WIDTH * WORLD_HEIGHT);
    }

    // Fill the terrain layer.
    // TODO: Load this from some storage format.
    SDL2pp::Rect floorPosInTexture{(256 * 6), 0, 256, 512};
    std::fill(mapLayers[0].begin(), mapLayers[0].end(),
              Sprite{inSpriteTexturePtr, floorPosInTexture, 256, 512});

    // Add some rugs to the 2nd layer.
    SDL2pp::Rect rugPosInTexture{(256 * 6), (512 * 1), 256, 512};
    Sprite rugSprite{inSpriteTexturePtr, rugPosInTexture, 256, 512};
    mapLayers[1][4] = rugSprite;
    mapLayers[1][14] = rugSprite;
    mapLayers[1][20] = rugSprite;
    mapLayers[1][32] = rugSprite;
    mapLayers[1][46] = rugSprite;
}

} // namespace Client
} // namespace AM
