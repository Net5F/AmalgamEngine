#include "TileExtent.h"
#include "ChunkExtent.h"
#include "SharedConfig.h"

namespace AM
{
TileExtent::TileExtent()
: DiscreteExtent<DiscreteImpl::TileTag>()
{
}

TileExtent::TileExtent(int inX, int inY, int inZ, int inXLength, int inYLength,
                       int inZLength)
: DiscreteExtent<DiscreteImpl::TileTag>(inX, inY, inZ, inXLength, inYLength,
                                        inZLength)
{
}

TileExtent::TileExtent(const ChunkExtent& chunkExtent)
: DiscreteExtent<DiscreteImpl::TileTag>(
    static_cast<int>(chunkExtent.x * SharedConfig::CHUNK_WIDTH),
    static_cast<int>(chunkExtent.y * SharedConfig::CHUNK_WIDTH), chunkExtent.z,
    static_cast<int>(chunkExtent.xLength * SharedConfig::CHUNK_WIDTH),
    static_cast<int>(chunkExtent.yLength * SharedConfig::CHUNK_WIDTH),
    chunkExtent.zLength)
{
}

} // End namespace AM
