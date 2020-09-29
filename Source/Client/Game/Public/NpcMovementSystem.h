#pragma once

#include "GameDefs.h"
#include "NetworkDefs.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "InputComponent.h"
#include <queue>

namespace AM
{

class EntityUpdate;

namespace Client
{

class Game;
class World;
class Network;

/**
 * Processes NPC (networked player and AI) entity update messages and moves their
 * entities appropriately.
 */
class NpcMovementSystem
{
public:
    /** Our best guess at a good amount of ticks in the past to replicate NPCs at. */
    static constexpr unsigned int PAST_TICK_OFFSET = 10;

    NpcMovementSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * If we've received data for the appropriate ticks, updates all NPCs.
     * Else, does no updates, leaving NPCs where they are.
     */
    void updateNpcs();

private:
    /** Represents what NPC changes happened on a single server tick. */
    struct NpcStateUpdate {
        /** The tick that this update refers to. */
        Uint32 tickNum = 0;
        /** Whether at least 1 NPC's state changed on this tick or not. */
        bool dataChanged = false;
        /** If dataChanged == true, contains the update message data. */
        const std::shared_ptr<const EntityUpdate>& entityUpdate = nullptr;
    };

    /**
     * Moves all NPCs using their current inputs.
     */
    void moveAllNpcs();

    /**
     * Receives NPC entity update messages from the network and pushes them into the
     * stateUpdateQueue.
     */
    void receiveEntityUpdates();

    /** Pushes a message confirming that a tick was processed with no update. */
    void handleExplicitConfirmation();
    /** Pushes messages confirming ticks up to confirmedTick. */
    void handleImplicitConfirmation(const Uint32 confirmedTick);
    /** Handles an update message, including implicit confirmations based on it. */
    void handleUpdate(const std::shared_ptr<const EntityUpdate>& entityUpdate);

    /**
     * Applies the given update message to the entity world state.
     */
    void applyUpdateMessage(const std::shared_ptr<const EntityUpdate>& entityUpdate);

    /** Holds NPC state deltas that are waiting to be processed. */
    std::queue<NpcStateUpdate> stateUpdateQueue;

    /** The latest tick that we've received an NPC update message for. */
    Uint32 lastReceivedTick;

    /** The last tick that we processed update data for. */
    Uint32 lastProcessedTick;

    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM
