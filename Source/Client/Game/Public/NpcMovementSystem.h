#ifndef NPCMOVEMENTSYSTEM_H
#define NPCMOVEMENTSYSTEM_H

#include "GameDefs.h"
#include "NetworkDefs.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "InputComponent.h"
#include "CircularBuffer.h"

namespace AM
{
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
    /*
     * The number of NPC entity update messages that we'll remember.
     * TODO: This is dependent on latency to the server, but CircularBuffer can't be resized
     *       at runtime. Modify CircularBuffer to be resizable and figure out where the
     *       size should be calculated.
     *       World::INPUT_HISTORY_LENGTH needs the same treatment.
     */
    static constexpr unsigned int UPDATE_MESSAGE_BUFFER_LENGTH = 10;

    /** Our best guess at a good amount of time in the past to replicate NPCs at. */
    static constexpr int PAST_TICK_OFFSET = -2;

    NpcMovementSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * If we've received data for the appropriate ticks, updates all NPCs.
     * Else, does no updates, leaving NPCs where they are.
     */
    void updateNpcs();

    /**
     * Receives NPC entity update messages from the network and pushes them into the
     * updateBuffer.
     * @return The number of received messages.
     */
    unsigned int receiveEntityUpdates();

    /**
     * Applies the given update message to the entity world state.
     */
    void applyUpdateMessage(const BinaryBufferSharedPtr& messageBuffer);

private:
    /** Holds NPC entity update message pointers. The message at updateBuffer[0] will
        always be for the tick value of latestReceivedTick. */
    CircularBuffer<BinaryBufferSharedPtr, UPDATE_MESSAGE_BUFFER_LENGTH> updateBuffer;

    /** The latest tick that we've received an NPC update message for. */
    Uint32 latestReceivedTick;

    /** The last tick that we processed update data for. */
    Uint32 lastProcessedTick;

    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM

#endif /* NPCMOVEMENTSYSTEM_H */
