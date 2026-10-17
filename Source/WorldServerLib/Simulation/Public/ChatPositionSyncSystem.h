#pragma once

#include "entt/fwd.hpp"
#include <SDL3/SDL_stdinc.h>
#include <vector>

namespace AM
{
namespace WorldServer
{
struct SimulationContext;
class Simulation;
class World;
class Network;

/**
 * Tells the chat server where client entities are, so it can route local
 * chat.
 *
 * Every Config::CHAT_POSITION_UPDATE_INTERVAL_S, sends the account IDs of any
 * client entities that were removed, followed by the position of every client
 * entity.
 *
 * A client entity is considered to be in the world for as long as it exists.
 * This includes the time that it lingers after its client disconnects.
 *
 * Note: We send every position, instead of only the ones that changed. This
 *       was measured to be cheaper than tracking changes, and means that we
 *       don't need to do anything special when we reconnect to the chat
 *       server (which forgets every position when we connect).
 */
class ChatPositionSyncSystem
{
public:
    ChatPositionSyncSystem(const SimulationContext& inSimContext);

    ~ChatPositionSyncSystem();

    /**
     * If an update is due, sends client entity positions to the chat server.
     */
    void sendPositionUpdates();

private:
    /**
     * Records the destroyed client entity's account as having left the world.
     */
    void onClientSimDataDestroyed(entt::registry& registry,
                                  entt::entity entity);

    /**
     * Sends the account IDs of any client entities that were removed since
     * our last update.
     */
    void sendPlayersLeftWorld();

    /**
     * Sends the position of every client entity.
     */
    void sendPlayerPositions();

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access components. */
    World& world;
    /** Used to send messages to the chat server. */
    Network& network;

    /** The account IDs of client entities that were removed since our last
        update.
        Note: When a client takes over a lingering entity, its ClientSimData
              is re-constructed, so the account will show up here even though
              its entity was never removed. This is fine, since we send
              departures before positions, so the chat server will re-add
              it. */
    std::vector<Sint64> leftAccountIDs;

    /** The tick that our next update should be sent on. */
    Uint32 nextUpdateTick;
};

} // End namespace WorldServer
} // End namespace AM
