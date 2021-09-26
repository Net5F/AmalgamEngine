#pragma once

#include "NetworkDefs.h"
#include "ChunkWireSnapshot.h"
#include <vector>

namespace AM
{
/**
 * Used by the server to stream chunks to a client.
 */
struct UpdateChunks {
public:
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE = MessageType::UpdateChunks;

    /** Used as a "we should never hit this" cap on the number of chunks that
        we send at once. Only checked in debug builds. */
    static constexpr unsigned int MAX_CHUNKS = 50;

    /** The chunks that the client should load. */
    std::vector<ChunkWireSnapshot> chunks;
};

template<typename S>
void serialize(S& serializer, UpdateChunks& chunkUpdates)
{
    serializer.container(chunkUpdates.chunks,
                         static_cast<std::size_t>(UpdateChunks::MAX_CHUNKS));
}

} // End namespace AM
