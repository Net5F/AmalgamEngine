#pragma once

#include "DiscretePosition.h"

namespace AM
{
class ChunkPosition;

/**
 * A strong type alias, describing the position of a particular map tile.
 */
class TilePosition : public DiscretePosition<DiscreteImpl::TileTag>
{
public:
    TilePosition();

    TilePosition(int inX, int inY);

    explicit TilePosition(const ChunkPosition& chunkPosition);
};

} // namespace AM
