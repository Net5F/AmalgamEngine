#pragma once

#include "EntityMover.h"
#include "PlayerMovementUpdate.h"
#include "QueuedEvents.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{
namespace Client
{
struct SimulationContext;
class Simulation;
class World;
class Network;

/**
 * Processes movement update messages for the local player and moves the
 * entity appropriately.
 *
 * Since the local player is simulated in the future (in relation to the
 * server), any situation where the local player's movement changes in a way
 * that we can't predict (buffs/debuffs, gear changes, etc), we potentially
 * will fall out of sync.
 * We soft-solve this by having the server "announce" changes instead of
 * immediately performing them. Instead of immediately applying the debuff, the
 * server must announce "this debuff will apply on tick X". This gives the
 * the client time to receive the message and apply the change on the same tick
 * as the server.
 * If the client receives the message late, a desync is unavoidable. The client
 * must request a resync and deal with whatever rubberbanding occurs.
 */
class LocalPlayerMovementSystem
{
public:
    LocalPlayerMovementSystem(const SimulationContext& inSimContext);

    /**
     * Processes the local player's movement for this tick.
     *
     * If we received any local player movement updates from the server,
     * applies them and replays inputs.
     */
    void processMovement();

private:
    /**
     * Processes any waiting local player updates (sent by the server).
     *
     * @return The tick number of the newest message that we received.
     */
    Uint32 processLocalPlayerUpdates();

    /**
     * Replay any inputs that are from newer ticks than lastUpdateTick.
     */
    void replayInputs(Uint32 lastUpdateTick);

    /**
     * Processes a single tick of local player movement.
     *
     * @param inputStates The input state to use to move the entity.
     *
     */
    void moveLocalPlayer(const Input::StateArr& inputStates);

    /**
     * Calls registry.patch() on the local player's Position component to
     * trigger any on_update callbacks that are connected to them.
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

    EventQueue<PlayerMovementUpdate> localPlayerMovementUpdateQueue;
};

} // namespace Client
} // namespace AM
