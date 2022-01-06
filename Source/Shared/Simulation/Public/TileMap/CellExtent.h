#pragma once

#include "DiscreteExtent.h"

namespace AM
{

/**
 * A strong type alias, describing an extent of spatial partitioning grid
 * cells.
 */
using CellExtent = DiscreteExtent<DiscreteImpl::CellTag>;

} // End namespace AM
