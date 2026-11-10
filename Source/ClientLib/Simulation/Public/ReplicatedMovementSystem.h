#pragma once

#include "EntityMover.h"
#include "QueuedEvents.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{
struct MovementUpdate;

namespace Client
{
struct SimulationContext;
class Simulation;
class World;
class Network;

/**
 * Processes movement update messages for replicated entities and moves them
 * appropriately.
 *
 * Note: "Replicated entities" are all entities that this client doesn't
 *       predict, i.e. everything except the local player (other player
 *       entities and all non-player entities). Their state is applied as
 *       received from the server, at a point in the past (see
 *       ReplicationTickOffset).
 */
class ReplicatedMovementSystem
{
public:
    ReplicatedMovementSystem(const SimulationContext& inSimContext);

    /**
     * If we've received data for the appropriate ticks, updates all replicated
     * entities. Else, does no updates, leaving replicated entities where they
     * are.
     */
    void updateReplicatedEntities();

private:
    /**
     * Initializes lastProcessedTick based on the sim's currentTick.
     */
    void initLastProcessedTick();

    /**
     * Moves all replicated entities using their current inputs.
     */
    void moveAllReplicatedEntities();

    /**
     * Applies the given update message to the entity world state.
     */
    void applyUpdateMessage(const MovementUpdate& movementUpdate);

    /**
     * Calls registry.patch() on each updated replicated entity's Position
     * component to trigger any on_update callbacks that are connected to them.
     * We don't patch until the end, because we may update the components
     * multiple times before we're done.
     *
     * Note: We only update Position because it's all we need right now. If
     *       the others are needed, they can be added.
     */
    void emitUpdateSignals();

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to send entity info request messages. */
    Network& network;

    EntityMover entityMover;

    EventQueue<std::shared_ptr<const MovementUpdate>> movementUpdateQueue;

    /** The last tick that we processed update data for. */
    Uint32 lastProcessedTick;
};

} // namespace Client
} // namespace AM
