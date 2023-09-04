#pragma once

#include "TilePosition.h"
#include "ChunkPosition.h"
#include "SharedConfig.h"

namespace AM
{
/**
 * Represents a position in the world.
 *
 * If used as a component attached to an entity, this will correspond to a 
 * a centered point under the entity's feet.
 * This means that an entity's sprite and bounding box will be centered on this
 * position in the X and Y axis, and will sit on top of this position in the Z
 * axis.
 *
 * We also commonly use this for 3D math.
 */
struct Position {
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
        float tileX{x / (static_cast<float>(SharedConfig::TILE_WORLD_WIDTH))};
        tileX = std::floor(tileX);
        float tileY{y / (static_cast<float>(SharedConfig::TILE_WORLD_WIDTH))};
        tileY = std::floor(tileY);

        return {static_cast<int>(tileX), static_cast<int>(tileY)};
    }

    /**
     * Returns the coordinates of the chunk that this position is within.
     */
    ChunkPosition asChunkPosition() const
    {
        float chunkX{(x / (static_cast<float>(SharedConfig::TILE_WORLD_WIDTH))
                      / static_cast<float>(SharedConfig::CHUNK_WIDTH))};
        chunkX = std::floor(chunkX);
        float chunkY{(y / (static_cast<float>(SharedConfig::TILE_WORLD_WIDTH))
                      / static_cast<float>(SharedConfig::CHUNK_WIDTH))};
        chunkY = std::floor(chunkY);

        return {static_cast<int>(chunkX), static_cast<int>(chunkY)};
    }

    /**
     * Returns the squared distance between this position and the given 
     * position.
     * We keep it squared to avoid an expensive sqrt. You can use this by 
     * squaring the distance you're comparing it to.
     */
    float squaredDistanceTo(const Position& other) const
    {
        float distanceX{std::abs(x - other.x)};
        float distanceY{std::abs(y - other.y)};
        float distanceZ{std::abs(z - other.z)};

        return {(distanceX * distanceX) + (distanceY * distanceY)
                + (distanceZ * distanceZ)};
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
