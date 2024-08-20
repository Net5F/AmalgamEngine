#include "CellExtent.h"
#include "TileExtent.h"
#include "BoundingBox.h"
#include "Cylinder.h"
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

    // Note: We use floor instead of integer division because negative values
    //       still need to round down.
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

CellExtent::CellExtent(const BoundingBox& box, float cellWidth,
                       float cellHeight)
{
    // Note: One could imagine doing the opposite logic here (if a box is 
    //       exactly touching the edge of a cell, include it in the extent).
    //       However, this would make it so that a box with the exact bounds 
    //       of a cell would convert into an extent that includes every cell 
    //       around it, which seems unexpected.

    // To account for float precision issues: add the epsilon to each min value,
    // then round down. If the box is within epsilon range of a cell in the 
    // negative direction, it won't be included in this extent.
    x = static_cast<int>(std::floor(
        (box.min.x + MovementHelpers::WORLD_EPSILON) / cellWidth));
    y = static_cast<int>(std::floor(
        (box.min.y + MovementHelpers::WORLD_EPSILON) / cellWidth));
    z = static_cast<int>(std::floor(
        (box.min.z + MovementHelpers::WORLD_EPSILON) / cellHeight));

    // Subtract the epsilon from each max value, then round up. If the box is 
    // within epsilon range of a cell in the positive direction, it won't be 
    // included in this extent.
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

CellExtent::CellExtent(const Cylinder& cylinder, float cellWidth,
                       float cellHeight)
{
    x = static_cast<int>(
        std::floor((cylinder.center.x - cylinder.radius) / cellWidth));
    y = static_cast<int>(
        std::floor((cylinder.center.y - cylinder.radius) / cellWidth));
    z = static_cast<int>(
        std::floor((cylinder.center.z - cylinder.halfHeight) / cellHeight));

    // Note: Be careful not to do this sort of thing in an initializer list 
    //       through the base class's constructor (we're subtracting other 
    //       struct members, which wouldn't yet be initialized).
    xLength = static_cast<int>(
                  std::ceil((cylinder.center.x + cylinder.radius) / cellWidth))
              - x;
    yLength = static_cast<int>(
                  std::ceil((cylinder.center.y + cylinder.radius) / cellWidth))
              - y;
    zLength = static_cast<int>(std::ceil(
                  (cylinder.center.z + cylinder.halfHeight) / cellHeight))
              - z;
}

void CellExtent::print() const
{
    LOG_INFO("(%d, %d, %d, %d, %d, %d)", x, y, z, xLength, yLength, zLength);
}

} // End namespace AM
