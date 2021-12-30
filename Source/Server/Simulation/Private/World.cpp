#include "World.h"
#include "ClientSimData.h"
#include "SharedConfig.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace Server
{
World::World(SpriteData& spriteData)
: registry()
, tileMap(spriteData)
, entityLocator(registry)
, device()
, generator(device())
// Note: We restrict the x-axis positions to keep them in bounds while moving.
, xDistribution(SharedConfig::TILE_WORLD_WIDTH
                , (static_cast<unsigned int>(SharedConfig::AOI_RADIUS)
                      - SharedConfig::TILE_WORLD_WIDTH))
, yDistribution(0, static_cast<unsigned int>(SharedConfig::AOI_RADIUS))
, baseX{0}
, baseY{0}
, groupOffsetsX{32, 64, 96, 128, 160, 32, 64, 96, 128, 160}
, groupOffsetsY{32, 32, 32, 32, 32, 64, 64, 64, 64, 64}
, offsetSelector{0}
{
    // Allocate the entity locator's grid.
    entityLocator.setGridSize(tileMap.getTileExtent().xLength,
        tileMap.getTileExtent().yLength);
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
        baseY += 400;
    }

    return position;
}

} // namespace Server
} // namespace AM
