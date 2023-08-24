#pragma once

#include "SpriteChange.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Client
{
class Simulation;
class World;
class Network;
class SpriteData;

/**
 * Requests needed tile map chunk data, and applies received chunk updates.
 */
class SpriteUpdateSystem
{
public:
    SpriteUpdateSystem(Simulation& inSimulation, World& inWorld,
                       Network& inNetwork, SpriteData& inSpriteData);

    /**
     * Processes any received sprite updates.
     */
    void updateSprites();

private:
    /** Used to get the current replication tick. */
    Simulation& simulation;
    /** Used to access the components we need to update. */
    World& world;
    /** Used to receive sprite update messages. */
    Network& network;
    /** Used to get sprite and sprite set data. */
    SpriteData& spriteData;

    EventQueue<SpriteChange> spriteChangeQueue;
};

} // namespace Client
} // namespace AM
