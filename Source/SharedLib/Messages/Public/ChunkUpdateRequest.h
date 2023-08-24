#pragma once

#include "EngineMessageType.h"
#include "ChunkPosition.h"
#include "NetworkDefs.h"
#include <vector>

namespace AM
{
/**
 * Used by the client to request chunk data.
 */
struct ChunkUpdateRequest {
public:
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ChunkUpdateRequest};

    /** Used as a "we should never hit this" cap on the number of chunks that
        we request at once. Only checked in debug builds. */
    static constexpr std::size_t MAX_CHUNKS{10};

    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** The chunks that the client is requesting. */
    std::vector<ChunkPosition> requestedChunks;

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
void serialize(S& serializer, ChunkUpdateRequest& chunkUpdateRequest)
{
    serializer.container(chunkUpdateRequest.requestedChunks,
                         ChunkUpdateRequest::MAX_CHUNKS);
}

} // End namespace AM
