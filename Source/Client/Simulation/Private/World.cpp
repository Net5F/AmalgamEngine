#include "World.h"
#include "Log.h"

namespace AM
{
namespace Client
{
World::World(const std::shared_ptr<SDL2pp::Texture>& inSpriteTexturePtr)
: playerEntity(entt::null)
, terrainMap(WORLD_WIDTH * WORLD_HEIGHT)
, mouseScreenPoint{}
{
    // Init the terrainMap.
    // TODO: Load this from some storage format.
    SDL2pp::Rect spritePosInTexture{(256 * 6), 0, 256, 512};
    std::fill(terrainMap.begin(), terrainMap.end(),
              Sprite{inSpriteTexturePtr, spritePosInTexture, 256, 512});
}

} // namespace Client
} // namespace AM
