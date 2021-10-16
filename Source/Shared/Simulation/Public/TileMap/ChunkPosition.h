#pragma once

#include <cmath>

namespace AM
{
/**
 * Holds the 2d position of a particular chunk.
 */
struct ChunkPosition {
public:
    // Note: Maps start at (0, 0) so we could make these unsigned, but these
    //       are signed to facilitate using this struct for things like
    //       negative offsets.
    int x{0};
    int y{0};

    bool operator==(const ChunkPosition& other)
    {
        return (x == other.x) && (y == other.y);
    }

    bool operator!=(const ChunkPosition& other)
    {
        return (x != other.x) || (y != other.y);
    }

    ChunkPosition operator+(const ChunkPosition& other)
    {
        return {(x + other.x), (y + other.y)};
    }

    ChunkPosition& operator+=(const ChunkPosition& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    bool isAdjacentTo(const ChunkPosition& other)
    {
        // Get the differences between the positions.
        int xDif{other.x - x};
        int yDif{other.y - y};

        // Square the differences.
        xDif *= xDif;
        yDif *= yDif;

        // Return the absolute squared distance.
        return (std::abs(xDif + yDif) <= 1);
    }
};

template<typename S>
void serialize(S& serializer, ChunkPosition& chunkPosition)
{
    serializer.value4b(chunkPosition.x);
    serializer.value4b(chunkPosition.y);
}

} // namespace AM
