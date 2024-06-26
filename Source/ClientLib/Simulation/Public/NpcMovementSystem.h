#pragma once

#include "EntityMover.h"
#include "QueuedEvents.h"
#include <SDL_stdinc.h>

namespace AM
{
struct MovementUpdate;

namespace Client
{
class Simulation;
class World;
class Network;

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
                      Network& inNetwork);

    /**
     * If we've received data for the appropriate ticks, updates all NPCs.
     * Else, does no updates, leaving NPCs where they are.
     */
    void updateNpcs();

private:
    /**
     * Initializes lastProcessedTick based on the sim's currentTick.
     */
    void initLastProcessedTick();

    /**
     * Moves all NPCs using their current inputs.
     */
    void moveAllNpcs();

    /**
     * Applies the given update message to the entity world state.
     */
    void applyUpdateMessage(const MovementUpdate& npcMovementUpdate);

    /**
     * Calls registry.patch() on each updated NPC's Position component to
     * trigger any on_update callbacks that are connected to them. We don't
     * patch until the end, because we may update the components multiple times
     * before we're done.
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

    EventQueue<std::shared_ptr<const MovementUpdate>> npcMovementUpdateQueue;

    /** The last tick that we processed update data for. */
    Uint32 lastProcessedTick;
};

} // namespace Client
} // namespace AM
