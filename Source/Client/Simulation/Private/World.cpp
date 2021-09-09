#include "World.h"
#include "AssetCache.h"
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
World::World(SpriteData& spriteData)
: playerEntity(entt::null)
, tileMap(spriteData)
, mouseScreenPoint{}
{
}

} // End namespace Client
} // End namespace AM
