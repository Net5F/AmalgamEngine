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

bool World::entityIDIsInUse(entt::entity entityID)
{
    // Note: We can't just check valid(), since calling create(N) will cause 
    //       valid() to return true for all X < N. We account for this by 
    //       checking if the index is actually in use.
    const auto& storage{registry.storage<entt::entity>()};
    if (registry.valid(entityID)
        && (storage.index(entityID) < storage.in_use())) {
        return true;
    }

    return false;
}

} // End namespace Client
} // End namespace AM
