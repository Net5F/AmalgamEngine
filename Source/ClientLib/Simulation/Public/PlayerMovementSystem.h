#pragma once

#include "EntityMover.h"
#include "PlayerMovementUpdate.h"
#include "QueuedEvents.h"
#include <SDL_stdinc.h>

namespace AM
{
namespace Client
{
class Simulation;
class World;
class Network;

/**
 * Processes movement update messages for the player's entity and moves the
 * entity appropriately.
 *
 * Since the player entity is simulated in the future (in relation to the 
 * server), any situation where the player's movement changes in a way that we
 * can't predict (buffs/debuffs, gear changes, etc), we potentially will fall 
 * out of sync.
 * We soft-solve this by having the server "announce" changes instead of 
 * immediately performing them. Instead of immediately applying the debuff, the 
 * server must announce "this debuff will apply on tick X". This gives the 
 * the client time to receive the message and apply the change on the same tick 
 * as the server.
 * If the client receives the message late, a desync is unavoidable. The client 
 * must request a resync and deal with whatever rubberbanding occurs.
 */
class PlayerMovementSystem
{
public:
    PlayerMovementSystem(Simulation& inSimulation, World& inWorld,
                         Network& inNetwork);

    /**
     * Processes the player entity's movement for this tick.
     *
     * If we received any player entity movement updates from the server,
     * applies them and replays inputs.
     */
    void processMovement();

private:
    /**
     * Processes any waiting player entity updates (sent by the server).
     *
     * @return The tick number of the newest message that we received.
     */
    Uint32 processPlayerUpdates();

    /**
     * Replay any inputs that are from newer ticks than lastUpdateTick.
     */
    void replayInputs(Uint32 lastUpdateTick);

    /**
     * Processes a single tick of player entity movement.
     *
     * @param inputStates The input state to use to move the entity.
     *                 
     */
    void movePlayerEntity(const Input::StateArr& inputStates);

    /**
     * Calls registry.patch() on the player's Position component to trigger
     * any on_update callbacks that are connected to them.
     * We don't patch until the end, because we may update the components
     * multiple times before we're done.
     *
     * Note: We only update Position because it's all we need right now. If
     *       the others are needed, they can be added.
     */
    void emitUpdateSignals();

    /** Debug printing, asserting. */
    void printMismatchInfo(Uint32 lastUpdateTick);
    void checkReceivedTickValidity(Uint32 updateTick, Uint32 currentTick);
    bool checkTickDiffValidity(Uint32 tickDiff);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to get the latest received tick. */
    Network& network;

    EntityMover entityMover;

    EventQueue<PlayerMovementUpdate> playerMovementUpdateQueue;
};

} // namespace Client
} // namespace AM
