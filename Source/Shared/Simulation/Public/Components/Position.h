#pragma once

#include "TilePosition.h"
#include "ChunkPosition.h"
#include "SharedConfig.h"

namespace AM
{
/**
 * Represents a position in the world.
 *
 * Generally, this will be a component attached to an entity, and will
 * correspond to the minimum x/y/z point (origin) of the entity's bounding box.
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
    TilePosition asTilePosition()
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
