#pragma once

#include "ClientNetworkDefs.h"
#include "NetworkDefs.h"
#include "QueuedEvents.h"
#include <queue>

namespace AM
{
class EntityUpdate;

namespace Client
{
class Simulation;
class World;
class Network;
class SpriteData;

/**
 * Processes NPC (networked player and AI) entity update messages and moves
 * their entities appropriately.
 */
class NpcMovementSystem
{
public:
    NpcMovementSystem(Simulation& inSim, World& inWorld, Network& inNetwork,
                      SpriteData& inSpriteData);

    /**
     * If we've received data for the appropriate ticks, updates all NPCs.
     * Else, does no updates, leaving NPCs where they are.
     */
    void updateNpcs();

    /**
     * Takes a tick adjustment from the server and applies it to NPC
     * replication through tickReplicationOffset.
     */
    void applyTickAdjustment(int adjustment);

private:
    /** Represents what NPC changes happened on a single server tick. */
    struct NpcStateUpdate {
        /** The tick that this update refers to. */
        Uint32 tickNum = 0;
        /** Whether at least 1 NPC's state changed on this tick or not. */
        bool dataChanged = false;
        /** If dataChanged == true, contains the update message data. */
        std::shared_ptr<const EntityUpdate> entityUpdate = nullptr;
    };

    /**
     * Moves all NPCs using their current inputs.
     */
    void moveAllNpcs();

    /**
     * Receives NPC entity update messages from the network and pushes them into
     * the stateUpdateQueue.
     */
    void receiveEntityUpdates();

    /** Pushes a message confirming that a tick was processed with no update. */
    void handleExplicitConfirmation();
    /** Pushes messages confirming ticks up to confirmedTick. */
    void handleImplicitConfirmation(const Uint32 confirmedTick);
    /** Handles an update message, including implicit confirmations based on it.
     */
    void handleUpdate(const std::shared_ptr<const EntityUpdate>& entityUpdate);

    /**
     * Applies the given update message to the entity world state.
     */
    void applyUpdateMessage(
        const std::shared_ptr<const EntityUpdate>& entityUpdate);

    /** Used to get the current tick. */
    Simulation& sim;
    /** Used to access components. */
    World& world;

    EventQueue<NpcUpdate> npcUpdateQueue;

    /** Temporarily used for loading NPC sprite data.
        When that logic gets moved, this member can be removed. */
    SpriteData& spriteData;

    /** Holds NPC state deltas that are waiting to be processed. */
    std::queue<NpcStateUpdate> stateUpdateQueue;

    /** The latest tick that we've received an NPC update message for. */
    Uint32 lastReceivedTick;

    /** The last tick that we processed update data for. */
    Uint32 lastProcessedTick;

    /**
     * How far into the past to replicate NPCs at.
     * e.g. If tickReplicationOffset == -5, on tick 15 we'll replicate NPC data
     *      for tick 10.
     *
     * Initialized to -2 * INITIAL_TICK_OFFSET (see applyTickAdjustment()
     * comment) and kept in line with sim tick adjustments.
     */
    int tickReplicationOffset;
};

} // namespace Client
} // namespace AM
