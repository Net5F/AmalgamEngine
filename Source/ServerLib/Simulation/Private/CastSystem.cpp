#include "CastSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "ItemData.h"
#include "CastableData.h"
#include "Inventory.h"
#include "Castable.h"
#include "CastState.h"
#include "CastCooldown.h"
#include "CastCooldownInit.h"
#include "ValidateCast.h"
#include "CastFailed.h"
#include "CastStarted.h"
#include "SaveTimestamp.h"
#include "EnttGroups.h"
#include "ClientSimData.h"
#include "Cylinder.h"
#include "SharedConfig.h"

namespace AM
{
namespace Server
{

CastSystem::CastSystem(Simulation& inSimulation, Network& inNetwork,
                       const ItemData& inItemData,
                       const CastableData& inCastableData)
: simulation{inSimulation}
, world{simulation.getWorld()}
, network{inNetwork}
, itemData{inItemData}
, castableData{inCastableData}
, playerCastCooldownObserver{}
, castRequestQueue{inNetwork.getEventDispatcher()}
{
    // Observe player CastCooldown construction events.
    playerCastCooldownObserver.bind(world.registry);
    playerCastCooldownObserver.on_construct<ClientSimData>()
        .on_construct<CastCooldown>();

    // Note: When CastCooldown is loaded from the DB, it gets initialized in 
    //       World::initTimerComponents.
}

void CastSystem::sendCastCooldownInits()
{
    // If a player CastCooldown was constructed, send the initial state to that
    // player.
    // Note: This may happen when the player first logs in, or when they first 
    //       cast a Castable with a cooldown (since we don't add CastCooldown 
    //       to every entity).
    for (entt::entity entity : playerCastCooldownObserver) {
        if (world.registry.all_of<ClientSimData, CastCooldown>(entity)) {
            continue;
        }
        auto [client, castCooldown]
            = world.registry.get<ClientSimData, CastCooldown>(entity);

        network.serializeAndSend(client.netID, CastCooldownInit{castCooldown});
    }

    playerCastCooldownObserver.clear();
}

void CastSystem::processCasts()
{
    // Process any waiting cast requests.
    processCastRequests();

    // Update ongoing casts.
    updateCasts();
}

void CastSystem::processCastRequests()
{
    CastRequest castRequest{};
    while (castRequestQueue.pop(castRequest)) {
        // Find the entity ID of the client that sent this request.
        auto it{world.netIDMap.find(castRequest.netID)};
        if (it == world.netIDMap.end()) {
            // Client doesn't exist (may have disconnected), skip this request.
            continue;
        }
        entt::entity clientEntity{it->second};

        // Try to perform the cast.
        CastFailureType failureType{CastFailureType::None};
        if (auto* type{
                std::get_if<ItemInteractionType>(&castRequest.castableID)}) {
            failureType = world.castHelper.castItemInteraction(
                {*type, clientEntity, castRequest.slotIndex,
                 castRequest.targetEntity, castRequest.targetPosition,
                 castRequest.netID});
        }
        else if (auto* type{
                std::get_if<EntityInteractionType>(&castRequest.castableID)}) {
            failureType = world.castHelper.castEntityInteraction(
                {*type, clientEntity, castRequest.targetEntity,
                 castRequest.targetPosition, castRequest.netID});
        }
        else if (auto* type{
                std::get_if<SpellType>(&castRequest.castableID)}) {
            failureType = world.castHelper.castSpell(
                {*type, clientEntity, castRequest.targetEntity,
                 castRequest.targetPosition, castRequest.netID});
        }
        else {
            LOG_ERROR("Tried to cast Castable with invalid type.");
        }

        // If the cast failed, send the failure to the caster.
        if (failureType != CastFailureType::None) {
            network.serializeAndSend<CastFailed>(
                castRequest.netID,
                {clientEntity, castRequest.castableID, failureType});
        }
    }
}

void CastSystem::updateCasts()
{
    auto view{world.registry.view<CastState>()};
    auto movementGroup{EnttGroups::getMovementGroup(world.registry)};
    Uint32 currentTick{simulation.getCurrentTick()};

    // Iterate each entity that is currently casting.
    for (auto [entity, castState] : view.each()) {
        // If this cast is unstarted, start it.
        if (castState.endTick == 0) {
            startCast(castState);
            continue;
        }
        // If the entity has moved, cancel the cast.
        else if (auto [position, prevPosition]
                 = movementGroup.get<Position, PreviousPosition>(entity);
                 position != prevPosition) {
            world.registry.erase<CastState>(entity);
        }
        // If the cast has finished, handle it.
        else if (currentTick == castState.endTick) {
            handleCast(castState.castInfo);
            world.registry.erase<CastState>(entity);
        }
    }
}

void CastSystem::startCast(CastState& castState)
{
    // Send a CastStarted to all nearby clients.
    sendCastStarted(castState);

    // Track the GCD.
    // Note: If CastCooldown gets created here, its lastUpdateTick will be set 
    //       to currentTick.
    Uint32 currentTick{simulation.getCurrentTick()};
    CastInfo& castInfo{castState.castInfo};
    CastCooldown& castCooldown{world.registry.get_or_emplace<CastCooldown>(
        castInfo.casterEntity, currentTick)};
    castCooldown.gcdTicksRemaining = SharedConfig::CAST_GLOBAL_COOLDOWN_TICKS;

    // If this castable has a cooldown, track it.
    if (castInfo.castable->cooldownTime > 0) {
        Uint32 castTimeTicks{
            static_cast<Uint32>(castInfo.castable->cooldownTime
                                / SharedConfig::SIM_TICK_TIMESTEP_S)};
        castCooldown.cooldowns.emplace_back(castInfo.castable->castableID,
                                            castTimeTicks);
    }

    // If this is an instant cast, handle it immediately.
    if (castInfo.castable->castTime == 0) {
        handleCast(castInfo);
    }
    else {
        // Not an instant cast. Set its end tick.
        Uint32 castTimeTicks{static_cast<Uint32>(
            castInfo.castable->castTime / SharedConfig::SIM_TICK_TIMESTEP_S)};
        castState.endTick = currentTick + castTimeTicks;
    }
}

void CastSystem::sendCastStarted(CastState& castState)
{
    // Serialize a CastStarted message.
    CastInfo& castInfo{castState.castInfo};
    CastStarted castStarted{.casterEntity{castInfo.casterEntity},
                            .castableID{castInfo.castable->castableID},
                            .itemID{castInfo.item->numericID},
                            .targetEntity{castInfo.targetEntity},
                            .targetPosition{castInfo.targetPosition}};
    BinaryBufferSharedPtr message{network.serialize(castStarted)};

    // Get the list of entities that are in range of the caster entity.
    const std::vector<entt::entity>* entitiesInRange{nullptr};
    if (const auto* client
        = world.registry.try_get<ClientSimData>(castInfo.casterEntity)) {
        // Clients already have their AOI list built.
        entitiesInRange = &(client->entitiesInAOI);
    }
    else {
        const auto& casterEntityPosition{
            world.registry.get<Position>(castInfo.casterEntity)};
        entitiesInRange = &(world.entityLocator.getEntities(
            Cylinder{casterEntityPosition, SharedConfig::AOI_RADIUS,
                     SharedConfig::AOI_HALF_HEIGHT}));
    }

    // Send the update to all nearby clients.
    auto view{world.registry.view<Position, ClientSimData>()};
    for (entt::entity entity : *entitiesInRange) {
        if (view.contains(entity)) {
            const auto& client{view.get<ClientSimData>(entity)};
            network.send(client.netID, message);
        }
    }
}

void CastSystem::handleCast(const CastInfo& castInfo)
{
    // Pass this cast to the appropriate handler.
    if (auto* type{std::get_if<ItemInteractionType>(
            &(castInfo.castable->castableID))}) {
        auto& map{world.castHelper.onItemInteractionCompletedMap};
        auto it{map.find(*type)};
        if (it != map.end()) {
            it->second(castInfo);
        }
    }
    else if (auto* type{std::get_if<EntityInteractionType>(
                 &(castInfo.castable->castableID))}) {
        auto& map{world.castHelper.onEntityInteractionCompletedMap};
        auto it{map.find(*type)};
        if (it != map.end()) {
            it->second(castInfo);
        }
    }
    else if (auto* type{
                 std::get_if<SpellType>(&(castInfo.castable->castableID))}) {
        auto& map{world.castHelper.onSpellCompletedMap};
        auto it{map.find(*type)};
        if (it != map.end()) {
            it->second(castInfo);
        }
    }
}

} // namespace Server
} // namespace AM
