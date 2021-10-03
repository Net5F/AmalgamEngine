#pragma once

#include "TileIndex.h"
#include "ChunkPosition.h"
#include "SharedConfig.h"

namespace AM
{
/**
 * Represents the top left point of an entity's position in the world.
 */
struct Position {
public:
    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** Current position. */
    float x{0};
    float y{0};
    float z{0};

    /**
     * Returns the coordinates of the tile that this position is within.
     */
    TileIndex asTileIndex()
    {
        return {(static_cast<int>(x / SharedConfig::TILE_WORLD_WIDTH)),
                (static_cast<int>(y / SharedConfig::TILE_WORLD_WIDTH))};
    }

    /**
     * Returns the coordinates of the chunk that this position is within.
     */
    ChunkPosition asChunkPosition()
    {
        return {(static_cast<int>((x / SharedConfig::TILE_WORLD_WIDTH)
                                  / SharedConfig::CHUNK_WIDTH)),
                (static_cast<int>((y / SharedConfig::TILE_WORLD_WIDTH)
                                  / SharedConfig::CHUNK_WIDTH))};
    }

    bool operator==(const Position& other)
    {
        return (x == other.x) && (y == other.y) && (z == other.z);
    }

    bool operator!=(const Position& other)
    {
        return (x != other.x) || (y != other.y) || (z != other.z);
    }
};

template<typename S>
void serialize(S& serializer, Position& position)
{
    serializer.value4b(position.x);
    serializer.value4b(position.y);
    serializer.value4b(position.z);
}

} // namespace AM
