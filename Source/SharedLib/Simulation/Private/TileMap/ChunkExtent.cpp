#include "ChunkExtent.h"
#include "TileExtent.h"
#include "SharedConfig.h"

namespace AM
{
ChunkExtent::ChunkExtent()
: DiscreteExtent<DiscreteImpl::ChunkTag>()
{
}

ChunkExtent::ChunkExtent(int inX, int inY, int inZ, int inXLength,
                         int inYLength, int inZLength)
: DiscreteExtent<DiscreteImpl::ChunkTag>(inX, inY, inZ, inXLength, inYLength,
                                         inZLength)
{
}

ChunkExtent::ChunkExtent(const TileExtent& tileExtent)
: DiscreteExtent<DiscreteImpl::ChunkTag>(
    static_cast<int>(tileExtent.x / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tileExtent.y / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tileExtent.z / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tileExtent.xLength / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tileExtent.yLength / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tileExtent.zLength / SharedConfig::CHUNK_WIDTH))
{
}

ChunkExtent ChunkExtent::fromMapLengths(Uint16 mapXLengthChunks,
                                        Uint16 mapYLengthChunks,
                                        Uint16 mapZLengthChunks)
{
    return {-(mapXLengthChunks / 2), -(mapYLengthChunks / 2), 0,
            mapXLengthChunks,        mapYLengthChunks,        mapZLengthChunks};
}

} // End namespace AM
