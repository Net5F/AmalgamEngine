#include "ChatPositionSyncSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Config.h"
#include "PlayerPositionsUpdate.h"
#include "PlayersLeftWorld.h"
#include "ClientSimData.h"
#include "Position.h"
#include "tracy/Tracy.hpp"
#include <algorithm>
#include <utility>

namespace AM
{
namespace WorldServer
{
ChatPositionSyncSystem::ChatPositionSyncSystem(
    const SimulationContext& inSimContext)
: simulation{inSimContext.simulation}
, world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, leftAccountIDs{}
, nextUpdateTick{0}
{
    // When a client entity is destroyed, record that it left the world.
    world.registry.on_destroy<ClientSimData>()
        .connect<&ChatPositionSyncSystem::onClientSimDataDestroyed>(this);
}

ChatPositionSyncSystem::~ChatPositionSyncSystem()
{
    world.registry.on_destroy<ClientSimData>()
        .disconnect<&ChatPositionSyncSystem::onClientSimDataDestroyed>(this);
}

void ChatPositionSyncSystem::sendPositionUpdates()
{
    ZoneScoped;

    // If it isn't time to send an update, do nothing.
    Uint32 currentTick{simulation.getCurrentTick()};
    if (currentTick < nextUpdateTick) {
        return;
    }
    nextUpdateTick = currentTick + Config::CHAT_POSITION_UPDATE_INTERVAL_TICKS;

    // Note: Departures must be sent first. See leftAccountIDs.
    sendPlayersLeftWorld();
    sendPlayerPositions();
}

void ChatPositionSyncSystem::onClientSimDataDestroyed(entt::registry& registry,
                                                      entt::entity entity)
{
    leftAccountIDs.push_back(registry.get<ClientSimData>(entity).accountID);
}

void ChatPositionSyncSystem::sendPlayersLeftWorld()
{
    // Send the departures, split into as many messages as necessary.
    for (std::size_t i{0}; i < leftAccountIDs.size();
         i += PlayersLeftWorld::MAX_PLAYERS_PER_MESSAGE) {
        std::size_t endIndex{
            std::min(leftAccountIDs.size(),
                     i + PlayersLeftWorld::MAX_PLAYERS_PER_MESSAGE)};
        network.chatEndpoint.send(PlayersLeftWorld{.accountIDs{
            leftAccountIDs.begin() + i, leftAccountIDs.begin() + endIndex}});
    }

    leftAccountIDs.clear();
}

void ChatPositionSyncSystem::sendPlayerPositions()
{
    // Gather every client entity's position, sending a message whenever one
    // fills up.
    auto view{world.registry.view<ClientSimData, Position>()};
    PlayerPositionsUpdate positionsUpdate{};
    positionsUpdate.playerPositions.reserve(std::min(
        view.size_hint(), PlayerPositionsUpdate::MAX_PLAYERS_PER_MESSAGE));
    for (auto [entity, client, position] : view.each()) {
        positionsUpdate.playerPositions.push_back({client.accountID, position});
        if (positionsUpdate.playerPositions.size()
            == PlayerPositionsUpdate::MAX_PLAYERS_PER_MESSAGE) {
            network.chatEndpoint.send(std::move(positionsUpdate));
            positionsUpdate = {};
        }
    }

    if (!(positionsUpdate.playerPositions.empty())) {
        network.chatEndpoint.send(std::move(positionsUpdate));
    }
}

} // End namespace WorldServer
} // End namespace AM
