#include "TilePosition.h"
#include "ChunkPosition.h"
#include "Position.h"
#include "SharedConfig.h"

namespace AM
{
TilePosition::TilePosition()
: DiscretePosition<DiscreteImpl::TileTag>()
{
}

TilePosition::TilePosition(int inX, int inY)
: DiscretePosition<DiscreteImpl::TileTag>(inX, inY)
{
}

TilePosition::TilePosition(const ChunkPosition& chunkPosition)
: DiscretePosition<DiscreteImpl::TileTag>(
    static_cast<int>(chunkPosition.x * SharedConfig::CHUNK_WIDTH),
    static_cast<int>(chunkPosition.y * SharedConfig::CHUNK_WIDTH))
{
}

Position TilePosition::getCenterPosition()
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float HALF_TILE{TILE_WIDTH / 2};

    return {((x * TILE_WIDTH) - HALF_TILE), ((y * TILE_WIDTH) - HALF_TILE), 0};
}

} // End namespace AM
