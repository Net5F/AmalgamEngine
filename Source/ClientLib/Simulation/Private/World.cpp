#include "World.h"
#include "EnttGroups.h"

namespace AM
{
namespace Client
{
World::World(GraphicData& graphicData)
: registry{}
, playerEntity{entt::null}
, entityLocator{registry}
, collisionLocator{}
, tileMap{graphicData, collisionLocator}
{
    // Initialize our entt groups, before anyone tries to use them.
    EnttGroups::init(registry);
}

} // End namespace Client
} // End namespace AM
