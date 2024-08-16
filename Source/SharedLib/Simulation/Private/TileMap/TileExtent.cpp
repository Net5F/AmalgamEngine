#include "TileExtent.h"
#include "ChunkExtent.h"
#include "BoundingBox.h"
#include "MinMaxBox.h"
#include "Vector3.h"
#include "MovementHelpers.h"
#include "SharedConfig.h"
#include "Log.h"

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

TileExtent::TileExtent(
    const DiscreteExtent<DiscreteImpl::TileTag>& tileExtent)
: DiscreteExtent<DiscreteImpl::TileTag>(tileExtent)
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

TileExtent::TileExtent(const BoundingBox& boundingBox)
: TileExtent(MinMaxBox(boundingBox))
{
}

TileExtent::TileExtent(const MinMaxBox& box)
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    // Note: One could imagine doing the opposite logic here (if a box is 
    //       exactly touching the edge of a tile, include it in the extent).
    //       However, this would make it so that a box with the exact bounds 
    //       of a tile would convert into an extent that includes every tile 
    //       around it, which seems unexpected.

    // To account for float precision issues: add the epsilon to each min value,
    // then round down. If the box is within epsilon range of a tile in the 
    // negative direction, it won't be included in this extent.
    x = static_cast<int>(
        std::floor((box.min.x + MovementHelpers::WORLD_EPSILON) / TILE_WIDTH));
    y = static_cast<int>(
        std::floor((box.min.y + MovementHelpers::WORLD_EPSILON) / TILE_WIDTH));
    z = static_cast<int>(
        std::floor((box.min.z + MovementHelpers::WORLD_EPSILON) / TILE_HEIGHT));

    // Subtract the epsilon from each max value, then round up. If the box is 
    // within epsilon range of a tile in the positive direction, it won't be 
    // included in this extent.
    float maxTileX{
        std::ceil((box.max.x - MovementHelpers::WORLD_EPSILON) / TILE_WIDTH)};
    float maxTileY{
        std::ceil((box.max.y - MovementHelpers::WORLD_EPSILON) / TILE_WIDTH)};
    float maxTileZ{
        std::ceil((box.max.z - MovementHelpers::WORLD_EPSILON) / TILE_HEIGHT)};
    xLength = (static_cast<int>(maxTileX) - x);
    yLength = (static_cast<int>(maxTileY) - y);
    zLength = (static_cast<int>(maxTileZ) - z);
}

bool TileExtent::contains(const BoundingBox& box) const
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    MinMaxBox extentMinMaxBox{{x * TILE_WIDTH, y * TILE_WIDTH, z * TILE_HEIGHT},
                              {(x + xLength) * TILE_WIDTH,
                               (y + yLength) * TILE_WIDTH,
                               (z + zLength) * TILE_HEIGHT}};
    BoundingBox extentBox{extentMinMaxBox};

    return extentBox.contains(box);
}

bool TileExtent::contains(const Vector3& worldPoint) const
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    MinMaxBox extentBox{{x * TILE_WIDTH, y * TILE_WIDTH, z * TILE_HEIGHT},
                        {(x + xLength) * TILE_WIDTH, (y + yLength) * TILE_WIDTH,
                         (z + zLength) * TILE_HEIGHT}};

    return (extentBox.min.x <= worldPoint.x)
           && (extentBox.max.x >= worldPoint.x)
           && (extentBox.min.y <= worldPoint.y)
           && (extentBox.max.y >= worldPoint.y)
           && (extentBox.min.z <= worldPoint.z)
           && (extentBox.max.z >= worldPoint.z);
}

void TileExtent::print() const
{
    LOG_INFO("(%d, %d, %d, %d, %d, %d)", x, y, z, xLength, yLength, zLength);
}

} // End namespace AM
