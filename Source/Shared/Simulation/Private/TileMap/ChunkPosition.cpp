#include "ChunkPosition.h"
#include "TilePosition.h"
#include "SharedConfig.h"

namespace AM
{

ChunkPosition::ChunkPosition()
: DiscretePosition<DiscreteImpl::ChunkTag>()
{
}

ChunkPosition::ChunkPosition(int inX, int inY)
: DiscretePosition<DiscreteImpl::ChunkTag>(inX, inY)
{
}

ChunkPosition::ChunkPosition(const TilePosition& tilePosition)
: DiscretePosition<DiscreteImpl::ChunkTag>(
    static_cast<int>(tilePosition.x / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tilePosition.y / SharedConfig::CHUNK_WIDTH))
{
}

} // End namespace AM
