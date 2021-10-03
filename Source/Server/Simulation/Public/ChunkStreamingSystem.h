#pragma once

#include "NetworkDefs.h"
#include "entt/fwd.hpp"

namespace AM
{
struct ChunkPosition;
struct ChunkUpdate;

namespace Server
{
class World;
class Network;
class TileMap;

/**
 * This system handles streaming chunk data to clients.
 *
 * A client may require chunks to be sent when it logs in, moves into a new
 * chunk, or teleports.
 */
class ChunkStreamingSystem
{
public:
    ChunkStreamingSystem(World& inWorld, Network& inNetwork);

    /**
     * Sends chunks to any client that needs them.
     */
    void sendChunks();

private:
    /**
     * Sends all chunks that are in range of the given index.
     *
     * @param currentChunk  The chunk that the client is now in.
     * @param netID  The network ID of the client to send chunks to.
     */
    void sendAllInRangeChunks(const ChunkPosition& currentChunk, NetworkID netID);

    /**
     * Determines which chunks the given client entity just got in range of
     * and sends them.
     *
     * @param previousChunk  The chunk that the client was previously in.
     * @param currentChunk  The chunk that the client is now in.
     * @param netID  The network ID of the client to send chunks to.
     */
    void sendNewInRangeChunks(const ChunkPosition& previousChunk,
                              const ChunkPosition& currentChunk, NetworkID netID);

    /**
     * Adds the given chunk to the given UpdateChunks message.
     *
     * @param chunkPosition  The position of the chunk to add.
     * @param chunkUpdate  The message struct to add the chunk to.
     */
    void addChunkToMessage(const ChunkPosition& chunkPosition,
                           ChunkUpdate& chunkUpdate);

    /** Used for fetching entity data. */
    entt::registry& registry;

    /** Used for fetching map data. */
    TileMap& tileMap;

    /** Used for sending chunks to clients. */
    Network& network;
};

} // End namespace Server
} // End namespace AM
