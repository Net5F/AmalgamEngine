#pragma once

#include "DiscretePosition.h"

namespace AM
{
class TilePosition;

/**
 * A strong type alias, describing the position of a particular spatial
 * partitioning grid cell.
 */
using CellPosition = DiscretePosition<DiscreteImpl::CellTag>;

} // namespace AM
