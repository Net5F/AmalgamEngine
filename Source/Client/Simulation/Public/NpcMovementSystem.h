#pragma once

#include "ClientNetworkDefs.h"
#include "NetworkDefs.h"
#include "QueuedEvents.h"
#include <queue>

namespace AM
{
struct EntityUpdate;

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
     * Applies the given tick adjustment (received from the server) to
     * tickReplicationOffset.
     */
    void applyTickAdjustment(int adjustment);

private:
    /** Represents what NPC movement changes happened on a single server
        tick. */
    struct NpcMovementUpdate {
        /** The tick that this update refers to. */
        Uint32 tickNum{0};
        /** True if at least 1 NPC's state changed on this tick. */
        bool dataChanged{false};
        /** If dataChanged == true, contains the update message data. */
        std::shared_ptr<const MovementUpdate> movementUpdate{nullptr};
    };

    /**
     * Moves all NPCs using their current inputs.
     */
    void moveAllNpcs();

    /**
     * Receives NPC movement update messages from the network and pushes them
     * into the stateUpdateQueue.
     */
    void receiveMovementUpdates();

    /** Pushes a message confirming that a tick was processed with no update. */
    void handleExplicitConfirmation();
    /** Pushes messages confirming ticks up to confirmedTick. */
    void handleImplicitConfirmation(const Uint32 confirmedTick);
    /** Handles an update message, including implicit confirmations based on it.
     */
    void
        handleUpdate(const std::shared_ptr<const MovementUpdate>& entityUpdate);

    /**
     * Applies the given update message to the entity world state.
     */
    void applyUpdateMessage(
        const std::shared_ptr<const MovementUpdate>& entityUpdate);

    /** Used to get the current tick. */
    Simulation& sim;
    /** Used to access components. */
    World& world;
    /** Used to send entity info request messages. */
    Network& network;

    EventQueue<NpcUpdate> npcUpdateQueue;

    /** Temporarily used for loading NPC sprite data.
        When that logic gets moved, this member can be removed. */
    SpriteData& spriteData;

    /** Holds NPC movement state updates that are waiting to be processed. */
    std::queue<NpcMovementUpdate> movementUpdateQueue;

    /** The latest tick that we've received an NPC update message for. */
    Uint32 lastReceivedTick;

    /** The last tick that we processed update data for. */
    Uint32 lastProcessedTick;

    /**
     * How far into the past to replicate NPCs at.
     * e.g. If tickReplicationOffset == -5, on tick 15 we'll replicate NPC data
     *      for tick 10.
     */
    int tickReplicationOffset;
};

} // namespace Client
} // namespace AM
