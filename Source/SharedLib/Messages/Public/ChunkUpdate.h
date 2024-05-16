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
        we request at once. 9 is for the current chunk and all surrounding chunks 
        in the X/Y directions, 20 is a large number to cap how many levels the 
        map can have in the Z direction. */
    static constexpr std::size_t MAX_CHUNKS{9 * 20};

    /** The chunks that the client should load. */
    std::vector<ChunkWireSnapshot> chunks;
};

template<typename S>
void serialize(S& serializer, ChunkUpdate& chunkUpdate)
{
    serializer.container(chunkUpdate.chunks, ChunkUpdate::MAX_CHUNKS);
}

} // End namespace AM
