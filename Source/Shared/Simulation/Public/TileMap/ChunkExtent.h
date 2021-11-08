#pragma once

#include "DiscreteExtent.h"

namespace AM
{

/**
 * A strong type alias, describing a range of map chunks.
 */
using ChunkExtent = DiscreteExtent<ChunkTag>;

// TODO: Can we change this to a class and add an asTileExtent()?

} // End namespace AM
