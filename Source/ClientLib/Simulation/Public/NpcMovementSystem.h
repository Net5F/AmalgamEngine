#pragma once

#include "NpcMovementUpdate.h"
#include "QueuedEvents.h"
#include <SDL_stdinc.h>

namespace AM
{
struct EntityUpdate;

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
     * Moves all NPCs using their current inputs.
     */
    void moveAllNpcs();

    /**
     * Applies the given update message to the entity world state.
     */
    void applyUpdateMessage(const NpcMovementUpdate& npcMovementUpdate);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to send entity info request messages. */
    Network& network;

    EventQueue<NpcMovementUpdate> npcMovementUpdateQueue;

    /** The last tick that we processed update data for. */
    Uint32 lastProcessedTick;
};

} // namespace Client
} // namespace AM
