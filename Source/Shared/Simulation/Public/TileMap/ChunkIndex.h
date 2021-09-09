#pragma once

namespace AM
{
/**
 * Holds the 2d index of a particular chunk.
 */
struct ChunkIndex {
public:
    // Note: Maps start at (0, 0) so we could make these unsigned, but these
    //       are signed to facilitate using this struct for things like
    //       negative offsets.
    int x{0};
    int y{0};

    bool operator==(const ChunkIndex& other)
    {
        return (x == other.x) && (y == other.y);
    }

    bool operator!=(const ChunkIndex& other)
    {
        return (x != other.x) || (y != other.y);
    }

    ChunkIndex operator+(const ChunkIndex& other)
    {
        return {(x + other.x), (y + other.y)};
    }

    ChunkIndex& operator+=(const ChunkIndex& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
};

} // namespace AM
