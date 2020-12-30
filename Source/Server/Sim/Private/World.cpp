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
, baseX{0}
, baseY{0}
, groupOffsetsX{-500, -250, 0, 100, 200, -500, -250, 0, 100, 200}
, groupOffsetsY{0, 0, 0, 0, 0, -100, -100, -100, -100, -100}
, offsetSelector{0}
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

Position World::getRandomSpawnPoint()
{
    return {xDistribution(generator), yDistribution(generator), 0};
}

Position World::getGroupedSpawnPoint()
{
    // Find the next position.
    Position position{};
    position.x = baseX + groupOffsetsX[offsetSelector];
    position.y = baseY + groupOffsetsY[offsetSelector];

    // Advance the offset selector.
    offsetSelector++;
    if (offsetSelector == 10) {
        offsetSelector = 0;

        // Move up to the next position.
        baseY -= 1000;
    }

    return position;
}

} // namespace Server
} // namespace AM
