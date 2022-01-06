#pragma once

namespace AM
{

namespace DiscreteImpl
{

/**
 * A tag type to express that a DiscreteRange or DiscretePosition is using
 * tiles as its unit.
 */
struct TileTag{};

/**
 * A tag type to express that a DiscreteRange or DiscretePosition is using
 * chunks as its unit.
 */
struct ChunkTag{};

/**
 * A tag type to express that a DiscreteRange or DiscretePosition is using
 * spatial partitioning grid cells as its unit.
 */
struct CellTag{};

}

} // End namespace AM
