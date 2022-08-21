#pragma once

#include "MessageType.h"
#include <vector>

namespace AM
{
/**
 * Used by the server to stream updated tile data to a client.
 */
struct TileUpdate {
public:
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE{MessageType::TileUpdate};

    /** The X coordinate of the tile to update. */
    int tileX{0};

    /** The Y coordinate of the tile to update. */
    int tileY{0};

    /** The numeric ID of each sprite layer in this tile. */
    std::vector<int> numericIDs{};
};

template<typename S>
void serialize(S& serializer, TileUpdate& tileUpdate)
{
    serializer.value4b(tileUpdate.tileX);
    serializer.value4b(tileUpdate.tileY);
    serializer.container4b(
        tileUpdate.numericIDs,
        static_cast<std::size_t>(SharedConfig::MAX_TILE_LAYERS));
}

} // End namespace AM
