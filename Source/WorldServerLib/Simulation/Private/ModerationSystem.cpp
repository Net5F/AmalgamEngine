#include "ModerationSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "WorldModerationResponse.h"
#include "DisconnectNotice.h"
#include "AccountRole.h"
#include "AccountRoleHelpers.h"
#include "Position.h"
#include "ClientSimData.h"
#include "Log.h"
#include <algorithm>
#include <cctype>

namespace AM
{
namespace WorldServer
{

ModerationSystem::ModerationSystem(
    const SimulationContext& inSimContext,
    ClientConnectionSystem& inClientConnectionSystem)
: world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, clientConnectionSystem{inClientConnectionSystem}
, noticeQueue{inSimContext.networkEventDispatcher}
, worldRequestQueue{inSimContext.networkEventDispatcher}
{
}

void ModerationSystem::processModeration()
{
    AccountModeratedNotice notice{};
    while (noticeQueue.pop(notice)) {
        processNotice(notice);
    }

    WorldModerationRequest request{};
    while (worldRequestQueue.pop(request)) {
        processWorldRequest(request);
    }
}

void ModerationSystem::processNotice(const AccountModeratedNotice& notice)
{
    entt::entity entity{findAccountEntity(notice.accountID)};
    if (entity == entt::null) {
        // The account isn't in the world. Nothing to do.
        return;
    }

    switch (notice.action) {
        case AccountModerationAction::Ban: {
            // If the account's client is connected, disconnect it.
            // Note: When the disconnect event arrives, ClientConnectionSystem
            //       will remove the entity immediately (skipping the linger
            //       time).
            ConnectionHandle clientHandle{
                world.registry.get<ClientSimData>(entity).clientHandle};
            if (clientHandle != NULL_CONNECTION_HANDLE) {
                network.clientEndpoint.disconnect(clientHandle,
                                                  DisconnectNotice::Banned);
                LOG_INFO("Disconnected banned client. AccountID: %lld",
                         static_cast<long long>(notice.accountID));
            }
            else {
                // The entity is lingering. Remove it now.
                clientConnectionSystem.removeLingeringEntity(notice.accountID);
                LOG_INFO("Removed banned account's lingering entity. "
                         "AccountID: %lld",
                         static_cast<long long>(notice.accountID));
            }
            break;
        }
        case AccountModerationAction::SetRole: {
            // Update the role. This gets replicated to the client.
            world.registry.patch<AccountRole>(
                entity, [&](AccountRole& accountRole) {
                    accountRole.roleID = notice.roleID;
                });
            break;
        }
        default: {
            break;
        }
    }
}

void ModerationSystem::processWorldRequest(
    const WorldModerationRequest& request)
{
    // Find the entity ID of the client that sent this request.
    entt::entity requesterEntity{world.getPlayerEntity(request.clientHandle)};
    if (requesterEntity == entt::null) {
        // Client doesn't exist (may have disconnected), skip this request.
        return;
    }

    // A correctly-behaving client will never send an invalid action.
    if (!isValidWorldModerationAction(request.action)) {
        LOG_WARN("Received invalid world moderation action: %u",
                 static_cast<unsigned int>(request.action));
        return;
    }

    switch (request.action) {
        case WorldModerationAction::Teleport: {
            sendWorldResponse(request, teleport(request, requesterEntity));
            break;
        }
        default: {
            break;
        }
    }
}

ModerationResult
    ModerationSystem::teleport(const WorldModerationRequest& request,
                               entt::entity requesterEntity)
{
    if (!(world.hasPermission(request.clientHandle,
                              AccountPermission::Teleport))) {
        return ModerationResult::NotPermitted;
    }

    // Find the target. If none was given, the requester is teleporting
    // themselves.
    entt::entity targetEntity{requesterEntity};
    if (!(request.targetUsername.empty())) {
        targetEntity = findConnectedPlayer(request.targetUsername);
        if (targetEntity == entt::null) {
            return ModerationResult::TargetNotOnline;
        }
    }

    // If the target is someone else, the requester must outrank them.
    if (targetEntity != requesterEntity) {
        const AccountRoleDef& requesterRole{
            AccountRoleHelpers::getRoleOrDefault(
                world.registry.get<AccountRole>(requesterEntity).roleID)};
        const AccountRoleDef& targetRole{AccountRoleHelpers::getRoleOrDefault(
            world.registry.get<AccountRole>(targetEntity).roleID)};
        if (!(requesterRole.outranks(targetRole))) {
            return ModerationResult::NotPermitted;
        }
    }

    // Find the destination: either another player's position, or the bottom
    // center of a tile.
    Vector3 destination{};
    if (!(request.destinationUsername.empty())) {
        entt::entity destinationEntity{
            findConnectedPlayer(request.destinationUsername)};
        if (destinationEntity == entt::null) {
            return ModerationResult::DestinationNotOnline;
        }
        destination = world.registry.get<Position>(destinationEntity);
    }
    else {
        // Note: teleportEntity() doesn't check bounds, so we have to.
        if (!(world.tileMap.getTileExtent().contains(
                request.destinationTile))) {
            return ModerationResult::InvalidDestination;
        }
        destination = request.destinationTile.getCenteredBottomPoint();
    }

    if (!(world.teleportEntity(targetEntity, destination))) {
        return ModerationResult::InternalError;
    }

    const ClientSimData& requesterClient{
        world.registry.get<ClientSimData>(requesterEntity)};
    const ClientSimData& targetClient{
        world.registry.get<ClientSimData>(targetEntity)};
    LOG_INFO("%s teleported %s to (%.2f, %.2f, %.2f).",
             requesterClient.username.c_str(), targetClient.username.c_str(),
             destination.x, destination.y, destination.z);

    return ModerationResult::Success;
}

void ModerationSystem::sendWorldResponse(const WorldModerationRequest& request,
                                         ModerationResult result)
{
    network.clientEndpoint.send(
        request.clientHandle,
        WorldModerationResponse{
            .action{request.action},
            .result{result},
            .targetUsername{request.targetUsername},
            .destinationUsername{request.destinationUsername},
            .destinationTile{request.destinationTile}});
}

entt::entity ModerationSystem::findConnectedPlayer(std::string_view username)
{
    auto equalsIgnoreCase = [](std::string_view a, std::string_view b) {
        return std::ranges::equal(a, b, [](unsigned char x, unsigned char y) {
            return std::tolower(x) == std::tolower(y);
        });
    };

    for (auto [entity, client] : world.registry.view<ClientSimData>().each()) {
        if ((client.clientHandle != NULL_CONNECTION_HANDLE)
            && equalsIgnoreCase(client.username, username)) {
            return entity;
        }
    }

    return entt::null;
}

entt::entity ModerationSystem::findAccountEntity(Sint64 accountID)
{
    for (auto [entity, client] : world.registry.view<ClientSimData>().each()) {
        if (client.accountID == accountID) {
            return entity;
        }
    }

    return entt::null;
}

} // End namespace WorldServer
} // End namespace AM
