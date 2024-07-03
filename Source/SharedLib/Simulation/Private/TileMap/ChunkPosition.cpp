#include "ChunkPosition.h"
#include "Vector3.h"
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

ChunkPosition::ChunkPosition(const Vector3& worldPoint)
: DiscretePosition<DiscreteImpl::ChunkTag>(
    static_cast<int>(std::floor(
        worldPoint.x / static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)
        / static_cast<float>(SharedConfig::CHUNK_WIDTH))),
    static_cast<int>(std::floor(
        worldPoint.y / static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)
        / static_cast<float>(SharedConfig::CHUNK_WIDTH))),
    static_cast<int>(std::floor(
        worldPoint.z / static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)
        / static_cast<float>(SharedConfig::CHUNK_WIDTH))))
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
