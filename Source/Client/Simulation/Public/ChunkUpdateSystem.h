#pragma once

#include "QueuedEvents.h"
#include "ChunkUpdate.h"
#include <SDL2/SDL_stdinc.h>

namespace AM
{
struct ChunkPosition;

namespace Client
{
class Simulation;
class World;
class Network;
class SpriteData;
class TileMap;

/**
 * Requests needed tile map chunk data, and applies received chunk updates.
 */
class ChunkUpdateSystem
{
public:
    ChunkUpdateSystem(Simulation& inSim, World& inWorld
                      , EventDispatcher& inNetworkEventDispatcher
                      , Network& inNetwork, SpriteData& inSpriteData);

    /**
     * Requests any needed chunk data and applies received chunk updates.
     */
    void updateChunks();

private:
    /**
     * Checks if we need new chunk data. If so, sends a ChunkUpdateRequest to
     * the server.
     */
    void requestNeededUpdates();

    /**
     * Requests all currently in-range chunks from the server.
     *
     * @param currentChunk  The chunk that we are currently in.
     */
    void requestAllInRangeChunks(const ChunkPosition& currentChunk);

    /**
     * Determines which chunks we just got in range of and requests them from
     * the server.
     *
     * @param previousChunk  The chunk that we were previously in.
     * @param currentChunk  The chunk that we are now in.
     */
    void requestNewInRangeChunks(const ChunkPosition& previousChunk,
                              const ChunkPosition& currentChunk);

    /**
     * Receives any waiting chunk updates from the queue and applies them
     * to our tile map.
     */
    void receiveAndApplyUpdates();

    /**
     * Applies the given chunk snapshot's state to our tile map.
     */
    void applyChunkSnapshot(const ChunkWireSnapshot& chunk);

    /** Used to get the current tick. */
    Simulation& sim;
    /** Used to access the player entity and components. */
    World& world;
    /** Used to send chunk update request messages. */
    Network& network;
    /** Used to access sprites while adding tiles to the map. */
    SpriteData& spriteData;

    EventQueue<std::shared_ptr<const ChunkUpdate>> chunkUpdateQueue;
};

} // namespace Client
} // namespace AM
