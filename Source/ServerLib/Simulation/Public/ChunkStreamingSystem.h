#pragma once

#include "NetworkDefs.h"
#include "QueuedEvents.h"
#include "ChunkDataRequest.h"
#include "ChunkPosition.h"

namespace AM
{
struct ChunkUpdate;
class Tile;
struct TileSnapshot;
struct ChunkWireSnapshot;

namespace Server
{
struct SimulationContext;
class World;
class Network;

/**
 * Handles streaming chunk data to clients.
 *
 * A client may require chunks to be sent when it logs in, moves into a new
 * chunk, or teleports.
 *
 * Note: We have no validation to see if client entities are in range of the
 *       requested chunks. Maybe add that once we get a permissions system.
 */
class ChunkStreamingSystem
{
public:
    ChunkStreamingSystem(const SimulationContext& inSimContext);

    /**
     * Processes chunk update requests, sending chunk data if the request is
     * valid.
     */
    void sendChunks();

private:
    /**
     * Send a chunk update, containing the chunks from the given request.
     */
    void sendChunkUpdate(const ChunkDataRequest& chunkDataRequest);

    /**
     * Adds the given chunk to the given ChunkUpdate message.
     *
     * @param chunkPosition  The position of the chunk to add.
     * @param chunkUpdate  The message struct to add the chunk to.
     */
    void addChunkToMessage(const ChunkPosition& chunkPosition,
                           ChunkUpdate& chunkUpdate);

    /** Used for fetching entity, component, and map data. */
    World& world;
    /** Used for receiving chunk requests and sending chunks to clients. */
    Network& network;

    EventQueue<ChunkDataRequest> chunkDataRequestQueue;
};

} // End namespace Server
} // End namespace AM
