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

CellPosition::CellPosition(
    const DiscretePosition<DiscreteImpl::CellTag>& cellPosition)
: DiscretePosition<DiscreteImpl::CellTag>(cellPosition)
{
}

CellPosition::CellPosition(const Vector3& worldPoint, float cellWidth,
                           float cellHeight)
: DiscretePosition<DiscreteImpl::CellTag>(
    static_cast<int>(std::floor(worldPoint.x / cellWidth)),
    static_cast<int>(std::floor(worldPoint.y / cellWidth)),
    static_cast<int>(std::floor(worldPoint.z / cellHeight)))
{
}

CellPosition::CellPosition(const TilePosition& tilePosition,
                           std::size_t cellWidthTiles,
                           std::size_t cellHeightTiles)
: DiscretePosition<DiscreteImpl::CellTag>(
    static_cast<int>(
        std::floor(tilePosition.x / static_cast<float>(cellWidthTiles))),
    static_cast<int>(
        std::floor(tilePosition.y / static_cast<float>(cellWidthTiles))),
    static_cast<int>(
        std::floor(tilePosition.z / static_cast<float>(cellHeightTiles))))
{
}

void CellPosition::print() const
{
    LOG_INFO("(%d, %d, %d)", x, y, z);
}

} // End namespace AM
