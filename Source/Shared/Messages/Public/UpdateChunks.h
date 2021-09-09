#pragma once

#include "ChunkWireSnapshot.h"
#include <vector>

namespace AM
{

/**
 * Used by the server to stream chunks to a client.
 */
struct UpdateChunks
{
public:
    /** Used as a "we should never hit this" cap on the number of chunks that
        we send at once. Only checked in debug builds. */
    static constexpr unsigned int MAX_CHUNKS = 50;

    /** The chunks that the client should load. */
    std::vector<ChunkWireSnapshot> chunks;
};

template<typename S>
void serialize(S& serializer, UpdateChunks& chunkUpdates)
{
    serializer.container(chunkUpdates.chunks, static_cast<std::size_t>(UpdateChunks::MAX_CHUNKS));
}

} // End namespace AM
