#include "World.h"
#include "SharedConfig.h"
#include <SDL_rect.h>

namespace AM
{
namespace Client
{
World::World(SpriteData& spriteData)
: registry{}
, itemData{}
, playerEntity{entt::null}
, tileMap{spriteData}
, entityLocator{registry}
{
}

} // End namespace Client
} // End namespace AM
