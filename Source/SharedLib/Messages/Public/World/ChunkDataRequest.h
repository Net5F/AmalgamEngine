#pragma once

#include "EngineMessageType.h"
#include "ChunkPosition.h"
#include "ChunkUpdate.h"
#include "ConnectionHandle.h"
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
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, ChunkDataRequest& chunkDataRequest)
{
    serializer.container(chunkDataRequest.requestedChunks,
                         ChunkUpdate::MAX_CHUNKS);
}

} // End namespace AM
