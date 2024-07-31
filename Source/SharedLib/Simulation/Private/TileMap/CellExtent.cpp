#include "CellExtent.h"
#include "TileExtent.h"
#include "BoundingBox.h"
#include "MinMaxBox.h"
#include "MovementHelpers.h"
#include "SharedConfig.h"
#include "Log.h"
#include <cmath>

namespace AM
{
CellExtent::CellExtent()
: DiscreteExtent<DiscreteImpl::CellTag>()
{
}

CellExtent::CellExtent(int inX, int inY, int inZ, int inXLength, int inYLength,
                       int inZLength)
: DiscreteExtent<DiscreteImpl::CellTag>(inX, inY, inZ, inXLength, inYLength,
                                        inZLength)
{
}

CellExtent::CellExtent(
    const DiscreteExtent<DiscreteImpl::CellTag>& cellExtent)
: DiscreteExtent<DiscreteImpl::CellTag>(cellExtent)
{
}

CellExtent::CellExtent(const TileExtent& tileExtent, std::size_t cellWidthTiles,
                       std::size_t cellHeightTiles)
{
    const float CELL_WIDTH{static_cast<float>(cellWidthTiles)};
    const float CELL_HEIGHT{static_cast<float>(cellHeightTiles)};

    x = static_cast<int>(std::floor(tileExtent.x / CELL_WIDTH));
    y = static_cast<int>(std::floor(tileExtent.y / CELL_WIDTH));
    z = static_cast<int>(std::floor(tileExtent.z / CELL_HEIGHT));

    int maxX{static_cast<int>(
        std::ceil((tileExtent.x + tileExtent.xLength) / CELL_WIDTH))};
    int maxY{static_cast<int>(
        std::ceil((tileExtent.y + tileExtent.yLength) / CELL_WIDTH))};
    int maxZ{static_cast<int>(
        std::ceil((tileExtent.z + tileExtent.zLength) / CELL_HEIGHT))};

    xLength = (maxX - x);
    yLength = (maxY - y);
    zLength = (maxZ - z);
}

CellExtent::CellExtent(const BoundingBox& boundingBox, float cellWidth,
                        float cellHeight)
: CellExtent(MinMaxBox(boundingBox), cellWidth, cellHeight)
{
}

CellExtent::CellExtent(const MinMaxBox& box, float cellWidth,
                       float cellHeight)
{
    // To account for float precision issues: add the epsilon to each min value,
    // then round down. If the value was within epsilon range of the integer 
    // above it, it'll end up rounded up.
    x = static_cast<int>(std::floor(
        (box.min.x + MovementHelpers::WORLD_EPSILON) / cellWidth));
    y = static_cast<int>(std::floor(
        (box.min.y + MovementHelpers::WORLD_EPSILON) / cellWidth));
    z = static_cast<int>(std::floor(
        (box.min.z + MovementHelpers::WORLD_EPSILON) / cellHeight));

    // Subtract the epsilon from each max value, then round up. If the value was 
    // within epsilon range of the integer below it, it'll end up rounded down.
    float maxTileX{
        std::ceil((box.max.x - MovementHelpers::WORLD_EPSILON) / cellWidth)};
    float maxTileY{
        std::ceil((box.max.y - MovementHelpers::WORLD_EPSILON) / cellWidth)};
    float maxTileZ{
        std::ceil((box.max.z - MovementHelpers::WORLD_EPSILON) / cellHeight)};
    xLength = (static_cast<int>(maxTileX) - x);
    yLength = (static_cast<int>(maxTileY) - y);
    zLength = (static_cast<int>(maxTileZ) - z);
}

void CellExtent::print() const
{
    LOG_INFO("(%d, %d, %d, %d, %d, %d)", x, y, z, xLength, yLength, zLength);
}

} // End namespace AM
