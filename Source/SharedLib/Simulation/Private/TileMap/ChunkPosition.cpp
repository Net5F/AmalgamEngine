#include "ChunkPosition.h"
#include "TilePosition.h"
#include "SharedConfig.h"

namespace AM
{
ChunkPosition::ChunkPosition()
: DiscretePosition<DiscreteImpl::ChunkTag>()
{
}

ChunkPosition::ChunkPosition(int inX, int inY, int inZ)
: DiscretePosition<DiscreteImpl::ChunkTag>(inX, inY, inZ)
{
}

ChunkPosition::ChunkPosition(const TilePosition& tilePosition)
: DiscretePosition<DiscreteImpl::ChunkTag>(
    static_cast<int>(tilePosition.x / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tilePosition.y / SharedConfig::CHUNK_WIDTH),
    tilePosition.z)
{
}

} // End namespace AM
