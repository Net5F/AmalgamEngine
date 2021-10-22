#pragma once

#include "NetworkDefs.h"
#include "QueuedEvents.h"
#include "ChunkUpdateRequest.h"

namespace AM
{
struct ChunkPosition;
struct ChunkUpdate;

namespace Server
{
class World;
class Network;

/**
 * This system handles streaming chunk data to clients.
 *
 * A client may require chunks to be sent when it logs in, moves into a new
 * chunk, or teleports.
 *
 * Note: We have no validation to see if client entities are in range of the
 *       requested chunks, but the worlds are all open source so it doesn't
 *       matter anyway. If someone wants to see the map, they can already get
 *       it from github.
 */
class ChunkStreamingSystem
{
public:
    ChunkStreamingSystem(World& inWorld, EventDispatcher& inNetworkEventDispatcher, Network& inNetwork);

    /**
     * Processes chunk update requests, sending chunk data if the request is
     * valid.
     */
    void sendChunks();

private:
    /**
     * Send a chunk update, containing the chunks from the given request.
     */
    void sendChunkUpdate(const ChunkUpdateRequest& chunkUpdateRequest);

    /**
     * Adds the given chunk to the given UpdateChunks message.
     *
     * @param chunkPosition  The position of the chunk to add.
     * @param chunkUpdate  The message struct to add the chunk to.
     */
    void addChunkToMessage(const ChunkPosition& chunkPosition,
                           ChunkUpdate& chunkUpdate);

    /** Used for fetching entity, component, and map data. */
    World& world;
    /** Used for sending chunks to clients. */
    Network& network;

    EventQueue<ChunkUpdateRequest> chunkUpdateRequestQueue;
};

} // End namespace Server
} // End namespace AM
