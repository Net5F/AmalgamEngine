#pragma once

#include "EngineMessageType.h"
#include "ChunkPosition.h"
#include "ChunkUpdate.h"
#include "NetworkID.h"
#include <vector>

namespace AM
{
/**
 * Used by the client to request chunk data.
 */
struct ChunkDataRequest {
public:
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ChunkDataRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The chunks that the client is requesting. */
    std::vector<ChunkPosition> requestedChunks;

    //--------------------------------------------------------------------------
    // Local data
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
void serialize(S& serializer, ChunkDataRequest& chunkDataRequest)
{
    serializer.container(chunkDataRequest.requestedChunks,
                         ChunkUpdate::MAX_CHUNKS);
}

} // End namespace AM
