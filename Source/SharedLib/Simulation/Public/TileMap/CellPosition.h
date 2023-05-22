#pragma once

#include "DiscretePosition.h"

namespace AM
{
struct TilePosition;

/**
 * A strong type alias, describing the position of a particular spatial
 * partitioning grid cell.
 */
using CellPosition = DiscretePosition<DiscreteImpl::CellTag>;

} // namespace AM
