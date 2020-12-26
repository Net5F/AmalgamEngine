#include "World.h"
#include "ClientSimData.h"
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
    auto clientView = registry.view<ClientSimData>();

    // Find the entt::entity associated with the popped NetworkID.
    for (entt::entity entity : clientView) {
        auto& clientSimData = clientView.get<ClientSimData>(entity);
        if (clientSimData.netID == networkID) {
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
