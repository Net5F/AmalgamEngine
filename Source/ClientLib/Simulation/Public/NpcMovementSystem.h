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
 * Processes NPC (non-player-controlled) entity update messages and moves their 
 * entities appropriately.
 *
 * Note: By "NPC", we mean any entity that isn't controlled by this client's 
 *       player. See IsClientEntity.h for more info.
 */
class NpcMovementSystem
{
public:
    NpcMovementSystem(Simulation& inSimulation, World& inWorld,
                      Network& inNetwork, SpriteData& inSpriteData);

    /**
     * If we've received data for the appropriate ticks, updates all NPCs.
     * Else, does no updates, leaving NPCs where they are.
     */
    void updateNpcs();

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
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to send entity info request messages. */
    Network& network;

    /** NPC state updates, received from the server. */
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
};

} // namespace Client
} // namespace AM
