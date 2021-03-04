#pragma once

namespace AM
{
namespace Client
{
/**
 * Holds the indices that identify a tile.
 *
 * The indices are in reference to the World's tile map.
 */
struct TileIndex {
public:
    int x{0};
    int y{0};
};

} // namespace Client
} // namespace AM
