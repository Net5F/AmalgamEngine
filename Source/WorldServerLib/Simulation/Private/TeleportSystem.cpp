#include "TeleportSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "AccountRole.h"
#include "AccountRoleHelpers.h"
#include "Position.h"
#include "ClientSimData.h"
#include "Log.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <variant>

namespace AM
{
namespace WorldServer
{

TeleportSystem::TeleportSystem(const SimulationContext& inSimContext)
: world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, teleportRequestQueue{inSimContext.networkEventDispatcher}
{
}

void TeleportSystem::processTeleportRequests()
{
    TeleportRequest teleportRequest{};
    while (teleportRequestQueue.pop(teleportRequest)) {
        processTeleportRequest(teleportRequest);
    }
}

void TeleportSystem::processTeleportRequest(const TeleportRequest& request)
{
    // Find the entity ID of the client that sent this request.
    entt::entity requesterEntity{world.getPlayerEntity(request.clientHandle)};
    if (requesterEntity == entt::null) {
        // Client doesn't exist (may have disconnected), skip this request.
        return;
    }

    // Process the request, and respond with the result.
    TeleportResponse response{.result{teleport(request, requesterEntity)},
                              .targetUsername{request.targetUsername},
                              .destination{request.destination}};
    network.clientEndpoint.send(request.clientHandle, response);
}

TeleportResponse::Result
    TeleportSystem::teleport(const TeleportRequest& request,
                             entt::entity requesterEntity)
{
    using Result = TeleportResponse::Result;

    const AccountRoleDef& requesterRole{AccountRoleHelpers::getRoleOrDefault(
        world.registry.get<AccountRole>(requesterEntity).roleID)};
    if (!(requesterRole.hasPermission(AccountPermission::Teleport))) {
        return Result::NotPermitted;
    }

    // Find the target. If none was given, the requester is teleporting
    // themselves.
    entt::entity targetEntity{requesterEntity};
    if (!(request.targetUsername.empty())) {
        targetEntity = findConnectedPlayer(request.targetUsername);
        if (targetEntity == entt::null) {
            return Result::TargetNotOnline;
        }
    }

    // If the target is someone else, the requester must outrank them.
    if (targetEntity != requesterEntity) {
        const AccountRoleDef& targetRole{AccountRoleHelpers::getRoleOrDefault(
            world.registry.get<AccountRole>(targetEntity).roleID)};
        if (!(requesterRole.outranks(targetRole))) {
            return Result::NotPermitted;
        }
    }

    // Find the destination: either another player's position, or the bottom
    // center of a tile.
    Vector3 destination{};
    if (const std::string* destinationUsername{
            std::get_if<std::string>(&(request.destination))}) {
        entt::entity destinationEntity{
            findConnectedPlayer(*destinationUsername)};
        if (destinationEntity == entt::null) {
            return Result::DestinationNotOnline;
        }
        destination = world.registry.get<Position>(destinationEntity);
    }
    else {
        // Note: teleportEntity() doesn't check bounds, so we have to.
        const TilePosition& destinationTile{
            std::get<TilePosition>(request.destination)};
        if (!(world.tileMap.getTileExtent().contains(destinationTile))) {
            return Result::InvalidDestination;
        }
        destination = destinationTile.getCenteredBottomPoint();
    }

    if (!(world.teleportEntity(targetEntity, destination))) {
        return Result::InternalError;
    }

    const ClientSimData& requesterClient{
        world.registry.get<ClientSimData>(requesterEntity)};
    const ClientSimData& targetClient{
        world.registry.get<ClientSimData>(targetEntity)};
    LOG_INFO("%s teleported %s to (%.2f, %.2f, %.2f).",
             requesterClient.username.c_str(), targetClient.username.c_str(),
             destination.x, destination.y, destination.z);

    return Result::Success;
}

entt::entity TeleportSystem::findConnectedPlayer(std::string_view username)
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

} // End namespace WorldServer
} // End namespace AM
