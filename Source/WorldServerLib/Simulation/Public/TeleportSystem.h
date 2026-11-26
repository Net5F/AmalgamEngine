#pragma once

#include "TeleportRequest.h"
#include "TeleportResponse.h"
#include "QueuedEvents.h"
#include "entt/fwd.hpp"
#include <string_view>

namespace AM
{
namespace WorldServer
{
struct SimulationContext;
class World;
class Network;

/**
 * Handles clients' requests to teleport themselves or another player (e.g.
 * through the /tp chat command).
 *
 * Teleporting requires the Teleport permission. Teleporting another player
 * also requires outranking them (see AccountRoleDef).
 */
class TeleportSystem
{
public:
    TeleportSystem(const SimulationContext& inSimContext);

    /**
     * Processes any teleport requests that clients have sent us.
     */
    void processTeleportRequests();

private:
    /**
     * Checks the given request against the requester's role. If it passes,
     * applies it. Either way, responds to the requester.
     */
    void processTeleportRequest(const TeleportRequest& request);

    /**
     * Teleports the given request's target to its destination, if the
     * requester is permitted to.
     */
    TeleportResponse::Result teleport(const TeleportRequest& request,
                                      entt::entity requesterEntity);

    /**
     * Returns the player entity with a connected client whose account has
     * the given username (case-insensitive), or entt::null if there isn't
     * one.
     */
    entt::entity findConnectedPlayer(std::string_view username);

    /** Used to access the player entities. */
    World& world;
    /** Used to send responses. */
    Network& network;
    EventQueue<TeleportRequest> teleportRequestQueue;
};

} // End namespace WorldServer
} // End namespace AM
