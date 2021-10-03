#pragma once

#include <SDL2/SDL_stdinc.h>
#include "QueuedEvents.h"
#include "ChunkUpdate.h"

namespace AM
{

namespace Client
{
class Simulation;
class World;
class Network;
class SpriteData;
class TileMap;

/**
 * Processes tile map chunk update messages and update the map chunks
 * appropriately.
 */
class ChunkUpdateSystem
{
public:
    ChunkUpdateSystem(Simulation& inSim, World& inWorld
                      , Network& inNetwork, SpriteData& inSpriteData);

    /**
     * Checks for ChunkUpdate events and updates the tile map.
     */
    void updateChunks();

private:
    /**
     * Applies the given chunk snapshot's state to our tile map.
     */
    void applyChunkSnapshot(const ChunkWireSnapshot& chunk);

    /** Used to get the current tick. */
    Simulation& sim;
    /** Used to access map chunks. */
    TileMap& tileMap;
    /** Used to access sprite while adding tiles to the map. */
    SpriteData& spriteData;

    EventQueue<std::shared_ptr<const ChunkUpdate>> chunkUpdateQueue;
};

} // namespace Client
} // namespace AM
