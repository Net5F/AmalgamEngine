#pragma once

#include "DiscretePosition.h"

namespace AM
{

/**
 * A strong type alias, describing the position of a particular map tile.
 */
using TilePosition = DiscretePosition<TileTag>;

} // namespace AM
