#pragma once

#include "MessageType.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Used by the server to stream updated tile data to a client.
 */
struct TileUpdate {
public:
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE = MessageType::TileUpdate;

    /** The X coordinate of the tile to update. */
    int tileX{0};

    /** The Y coordinate of the tile to update. */
    int tileY{0};

    /** The index of the sprite layer that should be modified. */
    Uint8 layerIndex{0};

    /** The new sprite's numeric ID. */
    int numericID{-1};
};

template<typename S>
void serialize(S& serializer, TileUpdate& tileUpdate)
{
    serializer.value4b(tileUpdate.tileX);
    serializer.value4b(tileUpdate.tileY);
    serializer.value1b(tileUpdate.layerIndex);
    serializer.value4b(tileUpdate.numericID);
}

} // End namespace AM
