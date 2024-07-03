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

TilePosition::TilePosition(int inX, int inY, int inZ)
: DiscretePosition<DiscreteImpl::TileTag>(inX, inY, inZ)
{
}

TilePosition::TilePosition(const Vector3& worldPoint)
: DiscretePosition<DiscreteImpl::TileTag>(
    static_cast<int>(std::floor(
        worldPoint.x / static_cast<float>(SharedConfig::TILE_WORLD_WIDTH))),
    static_cast<int>(std::floor(
        worldPoint.y / static_cast<float>(SharedConfig::TILE_WORLD_WIDTH))),
    static_cast<int>(std::floor(
        worldPoint.z / static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT))))
{
}

TilePosition::TilePosition(const ChunkPosition& chunkPosition)
: DiscretePosition<DiscreteImpl::TileTag>(
    static_cast<int>(chunkPosition.x * SharedConfig::CHUNK_WIDTH),
    static_cast<int>(chunkPosition.y * SharedConfig::CHUNK_WIDTH),
    chunkPosition.z)
{
}

Vector3 TilePosition::getOriginPoint() const
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    return {x * TILE_WIDTH, y * TILE_WIDTH, z * TILE_HEIGHT};
}

Vector3 TilePosition::getCenterPoint() const
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};
    static constexpr float HALF_WIDTH{TILE_WIDTH / 2.f};
    static constexpr float HALF_HEIGHT{TILE_HEIGHT / 2.f};

    return {((x * TILE_WIDTH) + HALF_WIDTH), ((y * TILE_WIDTH) + HALF_WIDTH),
            ((z * TILE_HEIGHT) + HALF_HEIGHT)};
}

Vector3 TilePosition::getCenteredBottomPoint() const
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};
    static constexpr float HALF_WIDTH{TILE_WIDTH / 2.f};

    return {((x * TILE_WIDTH) + HALF_WIDTH), ((y * TILE_WIDTH) + HALF_WIDTH),
            (z * TILE_HEIGHT)};
}

} // End namespace AM
