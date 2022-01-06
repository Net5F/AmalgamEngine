#include "TilePosition.h"
#include "ChunkPosition.h"
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

} // End namespace AM
