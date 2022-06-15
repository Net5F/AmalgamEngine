#include "ChunkExtent.h"
#include "TileExtent.h"
#include "SharedConfig.h"

namespace AM
{
ChunkExtent::ChunkExtent()
: DiscreteExtent<DiscreteImpl::ChunkTag>()
{
}

ChunkExtent::ChunkExtent(int inX, int inY, int inXLength, int inYLength)
: DiscreteExtent<DiscreteImpl::ChunkTag>(inX, inY, inXLength, inYLength)
{
}

ChunkExtent::ChunkExtent(const TileExtent& tileExtent)
: DiscreteExtent<DiscreteImpl::ChunkTag>(
    static_cast<int>(tileExtent.x / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tileExtent.y / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tileExtent.xLength / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tileExtent.yLength / SharedConfig::CHUNK_WIDTH))
{
}

} // End namespace AM
