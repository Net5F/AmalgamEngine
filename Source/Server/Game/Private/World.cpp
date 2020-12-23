#include "World.h"
#include "ClientState.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace Server
{
World::World()
: device()
, generator(device())
, xDistribution(0, (SCREEN_WIDTH - 128))
, yDistribution(0, (SCREEN_HEIGHT - 128))
{
}

entt::entity World::findEntityWithNetID(NetworkID networkID)
{
    auto clientView = registry.view<ClientState>();

    // Find the entt::entity associated with the popped NetworkID.
    for (entt::entity entity : clientView) {
        auto& clientState = clientView.get<ClientState>(entity);
        if (clientState.netID == networkID) {
            return entity;
        }
    }

    // Failed to find an entity associated with the given networkID.
    return entt::null;
}

Position World::getSpawnPoint()
{
    return {xDistribution(generator), yDistribution(generator), 0};
}

} // namespace Server
} // namespace AM
