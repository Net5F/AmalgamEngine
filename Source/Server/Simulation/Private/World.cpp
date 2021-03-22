#include "World.h"
#include "ClientSimData.h"
#include "SharedConfig.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace Server
{
World::World()
: device()
, generator(device())
, xDistribution(0, ((SharedConfig::WORLD_WIDTH - 1) * SharedConfig::TILE_WORLD_WIDTH))
, yDistribution(0, ((SharedConfig::WORLD_HEIGHT - 1) * SharedConfig::TILE_WORLD_HEIGHT))
, baseX{0}
, baseY{0}
, groupOffsetsX{32, 64, 96, 128, 160, 32, 64, 96, 128, 160}
, groupOffsetsY{32, 32, 32, 32, 32, 64, -100, -100, -100, -100}
, offsetSelector{0}
{
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
