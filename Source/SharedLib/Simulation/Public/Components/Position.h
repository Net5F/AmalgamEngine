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
 * correspond to a centered point under the entity's feet.
 * This means that an entity's sprite and bounding box will be centered on this
 * position in the X and Y axis, and will sit on top of this position in the Z
 * axis.
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
    TilePosition asTilePosition() const
    {
        return {(static_cast<int>(x / SharedConfig::TILE_WORLD_WIDTH)),
                (static_cast<int>(y / SharedConfig::TILE_WORLD_WIDTH))};
    }

    /**
     * Returns the coordinates of the chunk that this position is within.
     */
    ChunkPosition asChunkPosition() const
    {
        return {(static_cast<int>((x / SharedConfig::TILE_WORLD_WIDTH)
                                  / SharedConfig::CHUNK_WIDTH)),
                (static_cast<int>((y / SharedConfig::TILE_WORLD_WIDTH)
                                  / SharedConfig::CHUNK_WIDTH))};
    }

    Position operator+(const Position& other) const
    {
        return {(x + other.x), (y + other.y), (z + other.z)};
    }

    Position operator-(const Position& other) const
    {
        return {(x - other.x), (y - other.y), (z - other.z)};
    }

    Position& operator+=(const Position& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Position& operator-=(const Position& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    bool operator==(const Position& other) const
    {
        return (x == other.x) && (y == other.y) && (z == other.z);
    }

    bool operator!=(const Position& other) const
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
