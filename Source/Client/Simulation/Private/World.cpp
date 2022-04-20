#include "World.h"
#include "SharedConfig.h"

#include <SDL_rect.h>

namespace AM
{
namespace Client
{
World::World(SpriteData& spriteData)
: registry()
, playerEntity(entt::null)
, tileMap(spriteData)
, mouseScreenPoint{}
{
}

} // End namespace Client
} // End namespace AM
