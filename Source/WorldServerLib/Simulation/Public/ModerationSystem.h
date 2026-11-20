#pragma once

#include "WorldModerationRequest.h"
#include "AccountModeratedNotice.h"
#include "QueuedEvents.h"
#include "entt/fwd.hpp"
#include <SDL3/SDL_stdinc.h>
#include <string_view>

namespace AM
{
namespace WorldServer
{
struct SimulationContext;
class World;
class Network;
class ClientConnectionSystem;

/**
 * Handles moderation within this world.
 *
 * Account-level actions (bans, role changes) are applied by AccountServer.
 * When it tells us that an account was moderated, we apply it to that
 * account's player entity (disconnecting its client, or updating its
 * AccountRole).
 *
 * World-level actions (e.g. teleporting) are requested by clients through
 * WorldModerationRequest. We check them against the requester's role and
 * apply them ourselves.
 */
class ModerationSystem
{
public:
    ModerationSystem(const SimulationContext& inSimContext,
                     ClientConnectionSystem& inClientConnectionSystem);

    /**
     * Processes AccountServer's moderation notices and clients' world
     * moderation requests.
     */
    void processModeration();

private:
    /**
     * Applies the given moderation action to the account's player entity.
     */
    void processNotice(const AccountModeratedNotice& notice);

    /**
     * Checks the given request against the requester's role. If it passes,
     * applies it. Either way, responds to the requester.
     */
    void processWorldRequest(const WorldModerationRequest& request);

    /**
     * Sends the given result to the requester of the given request.
     */
    void sendWorldResponse(const WorldModerationRequest& request,
                           ModerationResult result);

    /**
     * Returns the player entity with a connected client whose account has
     * the given username (case-insensitive), or entt::null if there isn't
     * one.
     */
    entt::entity findConnectedPlayer(std::string_view username);

    /**
     * Returns the player entity that belongs to the given account (including
     * lingering entities), or entt::null if there isn't one.
     */
    entt::entity findAccountEntity(Sint64 accountID);

    /** Used to access the player entities. */
    World& world;
    /** Used to send responses and disconnect clients. */
    Network& network;
    /** Used to remove banned accounts' lingering entities. */
    ClientConnectionSystem& clientConnectionSystem;
    EventQueue<AccountModeratedNotice> noticeQueue;
    EventQueue<WorldModerationRequest> worldRequestQueue;
};

} // End namespace WorldServer
} // End namespace AM
