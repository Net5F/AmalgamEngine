#include "TileExtent.h"
#include "ChunkExtent.h"
#include "CellExtent.h"
#include "BoundingBox.h"
#include "Cylinder.h"
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

TileExtent::TileExtent(const DiscreteExtent<DiscreteImpl::TileTag>& tileExtent)
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

TileExtent::TileExtent(const CellExtent& cellExtent, std::size_t cellWidthTiles,
                       std::size_t cellHeightTiles)
: DiscreteExtent<DiscreteImpl::TileTag>(
    static_cast<int>(cellExtent.x * cellWidthTiles),
    static_cast<int>(cellExtent.y * cellWidthTiles),
    static_cast<int>(cellExtent.z * cellHeightTiles),
    static_cast<int>(cellExtent.xLength * cellWidthTiles),
    static_cast<int>(cellExtent.yLength * cellWidthTiles),
    static_cast<int>(cellExtent.zLength * cellHeightTiles))
{
}

TileExtent::TileExtent(const BoundingBox& boundingBox)
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
    x = static_cast<int>(std::floor(
        (boundingBox.min.x + MovementHelpers::WORLD_EPSILON) / TILE_WIDTH));
    y = static_cast<int>(std::floor(
        (boundingBox.min.y + MovementHelpers::WORLD_EPSILON) / TILE_WIDTH));
    z = static_cast<int>(std::floor(
        (boundingBox.min.z + MovementHelpers::WORLD_EPSILON) / TILE_HEIGHT));

    // Subtract the epsilon from each max value, then round up. If the
    // boundingBox is within epsilon range of a tile in the positive direction,
    // it won't be included in this extent.
    float maxTileX{std::ceil(
        (boundingBox.max.x - MovementHelpers::WORLD_EPSILON) / TILE_WIDTH)};
    float maxTileY{std::ceil(
        (boundingBox.max.y - MovementHelpers::WORLD_EPSILON) / TILE_WIDTH)};
    float maxTileZ{std::ceil(
        (boundingBox.max.z - MovementHelpers::WORLD_EPSILON) / TILE_HEIGHT)};
    xLength = (static_cast<int>(maxTileX) - x);
    yLength = (static_cast<int>(maxTileY) - y);
    zLength = (static_cast<int>(maxTileZ) - z);
}

bool TileExtent::contains(const BoundingBox& box) const
{
    BoundingBox extentBox(*this);
    return extentBox.contains(box);
}

bool TileExtent::contains(const Cylinder& cylinder) const
{
    BoundingBox extentBox(*this);
    return extentBox.contains(cylinder);
}

bool TileExtent::contains(const Vector3& worldPoint) const
{
    BoundingBox extentBox(*this);
    return extentBox.contains(worldPoint);
}

void TileExtent::print() const
{
    LOG_INFO("(%d, %d, %d, %d, %d, %d)", x, y, z, xLength, yLength, zLength);
}

} // End namespace AM
