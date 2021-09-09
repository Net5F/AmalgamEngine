#pragma once

namespace AM
{
/**
 * Holds the 2d index of a particular tile.
 */
struct TileIndex {
public:
    // Note: Maps start at (0, 0) so we could make these unsigned, but these
    //       are signed to facilitate using this struct for things like
    //       negative offsets.
    int x{0};
    int y{0};
};

} // namespace AM
