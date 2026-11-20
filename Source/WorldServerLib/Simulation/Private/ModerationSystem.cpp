#include "ModerationSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "WorldModerationResponse.h"
#include "DisconnectNotice.h"
#include "AccountRole.h"
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
            // TODO: Implement teleporting:
            //   1. Check that the requester has the needed permission (add one
            //      to AccountPermission), using world.hasPermission(). If they
            //      don't, respond with NotPermitted.
            //   2. Find the target with findConnectedPlayer(). If they aren't
            //      in the world, respond with TargetNotOnline.
            //   3. If the target isn't the requester, check that the
            //      requester outranks them (AccountRoleHelpers::canActOn()).
            //   4. Teleport the target with world.teleportEntity(), and
            //      respond with Success.
            // Until then, nobody is permitted to teleport.
            sendWorldResponse(request, ModerationResult::NotPermitted);
            break;
        }
        default: {
            break;
        }
    }
}

void ModerationSystem::sendWorldResponse(const WorldModerationRequest& request,
                                         ModerationResult result)
{
    network.clientEndpoint.send(
        request.clientHandle,
        WorldModerationResponse{.action{request.action},
                                .result{result},
                                .targetUsername{request.targetUsername}});
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
