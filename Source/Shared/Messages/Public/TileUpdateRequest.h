#pragma once

#include "MessageType.h"
#include "NetworkDefs.h"
#include <SDL2/SDL_stdinc.h>

namespace AM
{
/**
 * Used by the server to stream updated tile data to a client.
 */
struct TileUpdateRequest {
public:
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE = MessageType::TileUpdateRequest;

    /** The X coordinate of the tile to update. */
    unsigned int tileX{0};

    /** The Y coordinate of the tile to update. */
    unsigned int tileY{0};

    /** The index of the sprite layer that should be modified. */
    Uint8 layerIndex{0};

    /** The new sprite's numeric ID. */
    int numericID{-1};

    //--------------------------------------------------------------------------
    // Non-replicated data
    //--------------------------------------------------------------------------
    /**
     * The network ID of the client that sent this message.
     * Set by the server.
     * No IDs are accepted from the client because we can't trust it,
     * so we fill in the ID based on which socket the message came from.
     */
    NetworkID netID{0};
};

template<typename S>
void serialize(S& serializer, TileUpdateRequest& tileUpdateRequest)
{
    serializer.value4b(tileUpdateRequest.tileX);
    serializer.value4b(tileUpdateRequest.tileY);
    serializer.value1b(tileUpdateRequest.layerIndex);
    serializer.value4b(tileUpdateRequest.numericID);
}

} // End namespace AM
