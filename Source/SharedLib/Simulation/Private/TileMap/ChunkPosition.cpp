#include "ChunkPosition.h"
#include "Vector3.h"
#include "TilePosition.h"
#include "SharedConfig.h"
#include "Log.h"

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

ChunkPosition::ChunkPosition(
    const DiscretePosition<DiscreteImpl::ChunkTag>& chunkPosition)
: DiscretePosition<DiscreteImpl::ChunkTag>(chunkPosition)
{
}

ChunkPosition::ChunkPosition(const Vector3& worldPoint)
{
    static constexpr float CHUNK_WIDTH{static_cast<float>(
        SharedConfig::TILE_WORLD_WIDTH * SharedConfig::CHUNK_WIDTH)};
    static constexpr float CHUNK_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    x = static_cast<int>(std::floor(worldPoint.x / CHUNK_WIDTH));
    y = static_cast<int>(std::floor(worldPoint.y / CHUNK_WIDTH));
    z = static_cast<int>(std::floor(worldPoint.z / CHUNK_HEIGHT));
}

ChunkPosition::ChunkPosition(const TilePosition& tilePosition)
: DiscretePosition<DiscreteImpl::ChunkTag>(
    static_cast<int>(tilePosition.x / SharedConfig::CHUNK_WIDTH),
    static_cast<int>(tilePosition.y / SharedConfig::CHUNK_WIDTH),
    tilePosition.z)
{
}

void ChunkPosition::print() const
{
    LOG_INFO("(%d, %d, %d)", x, y, z);
}

} // End namespace AM
