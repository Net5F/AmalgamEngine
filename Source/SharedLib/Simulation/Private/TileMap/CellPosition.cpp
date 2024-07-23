#include "CellPosition.h"
#include "Vector3.h"
#include "TilePosition.h"
#include "SharedConfig.h"
#include "MovementHelpers.h"
#include "Log.h"

namespace AM
{
CellPosition::CellPosition()
: DiscretePosition<DiscreteImpl::CellTag>()
{
}

CellPosition::CellPosition(int inX, int inY, int inZ)
: DiscretePosition<DiscreteImpl::CellTag>(inX, inY, inZ)
{
}

CellPosition::CellPosition(const Vector3& worldPoint)
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};

    // To account for float precision issues: add the epsilon to each value in 
    // the point, then round down. If the value was within epsilon range of the
    // integer above it, it'll end up rounded up.
    x = static_cast<int>(std::floor(
        (worldPoint.x + MovementHelpers::WORLD_EPSILON) / TILE_WIDTH));
    y = static_cast<int>(std::floor(
        (worldPoint.y + MovementHelpers::WORLD_EPSILON) / TILE_WIDTH));
    z = static_cast<int>(std::floor(
        (worldPoint.z + MovementHelpers::WORLD_EPSILON) / TILE_HEIGHT));
}

CellPosition::CellPosition(const TilePosition& tilePosition,
                           std::size_t cellWidth, std::size_t cellHeight)
: DiscretePosition<DiscreteImpl::CellTag>(
    static_cast<int>(
        std::floor(tilePosition.x / static_cast<float>(cellWidth))),
    static_cast<int>(
        std::floor(tilePosition.y / static_cast<float>(cellWidth))),
    static_cast<int>(
        std::floor(tilePosition.z / static_cast<float>(cellHeight))))
{
}

void CellPosition::print() const
{
    LOG_INFO("(%d, %d, %d)", x, y, z);
}

} // End namespace AM
