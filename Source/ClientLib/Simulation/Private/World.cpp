#include "World.h"
#include "SharedConfig.h"
#include <SDL_rect.h>

namespace AM
{
namespace Client
{
World::World(GraphicData& graphicData)
: registry{}
, itemData{}
, playerEntity{entt::null}
, tileMap{graphicData}
, entityLocator{registry}
{
}

} // End namespace Client
} // End namespace AM
