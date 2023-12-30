#pragma once

#include "EngineMessageType.h"
#include "ChunkWireSnapshot.h"
#include <vector>

namespace AM
{
/**
 * Used by the server to stream chunks to a client.
 */
struct ChunkUpdate {
public:
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ChunkUpdate};

    /** Used as a "we should never hit this" cap on the number of chunks that
        we send at once. Only checked in debug builds. */
    static constexpr std::size_t MAX_CHUNKS{50};

    /** The chunks that the client should load. */
    std::vector<ChunkWireSnapshot> chunks;
};

template<typename S>
void serialize(S& serializer, ChunkUpdate& chunkUpdate)
{
    serializer.container(chunkUpdate.chunks, ChunkUpdate::MAX_CHUNKS);
}

} // End namespace AM
