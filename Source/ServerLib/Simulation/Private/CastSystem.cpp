#include "CastSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
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

CastSystem::CastSystem(const SimulationContext& inSimContext)
: simulation{inSimContext.simulation}
, world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, itemData{inSimContext.itemData}
, castableData{inSimContext.castableData}
, playerCastCooldownObserver{}
, castRequestQueue{inSimContext.networkEventDispatcher}
, castRequestSorter{}
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
        if (!(world.registry.all_of<ClientSimData, CastCooldown>(entity))) {
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
    // Sort or process any waiting cast requests.
    while (CastRequest* castRequest{castRequestQueue.peek()}) {
        // If the castable doesn't exist, drop this request.
        const Castable* castable{
            castableData.getCastable(castRequest->castableID)};
        if (!castable) {
            castRequestQueue.pop();
            continue;
        }

        // If this castable is tick-synchronized, sort the request.
        if (castable->isTickSynchronized) {
            // Push the cast request into the sorter.
            SorterBase::ValidityResult result{
                castRequestSorter.push(*castRequest, castRequest->tickNum)};

            // If the cast request was late, process it immediately (it may 
            // still succeed).
            if (result != SorterBase::ValidityResult::Valid) {
                processCastRequest(*castRequest);
            }
            else {
                castRequestQueue.pop();
                continue;
            }
        }
        else {
            // Not tick-synchronized. Process it immediately.
            processCastRequest(*castRequest);
        }

        castRequestQueue.pop();
    }

    // Process all cast requests for this tick.
    std::queue<CastRequest>& queue{castRequestSorter.getCurrentQueue()};
    for (; !(queue.empty()); queue.pop()) {
        processCastRequest(queue.front());
    }

    // Advance the sorter to the next tick.
    castRequestSorter.advance();
}

void CastSystem::processCastRequest(const CastRequest& castRequest)
{
    // Find the entity ID of the client that sent this request.
    entt::entity clientEntity{world.getClientEntity(castRequest.netID)};
    if (clientEntity == entt::null) {
        // Client doesn't exist (may have disconnected), skip this request.
        return;
    }

    // Try to perform the cast.
    CastFailureType result{CastFailureType::None};
    if (auto* type{
            std::get_if<ItemInteractionType>(&castRequest.castableID)}) {
        result = world.castHelper.castItemInteraction(
            {*type, clientEntity, castRequest.slotIndex,
             castRequest.targetEntity, castRequest.targetPosition,
             castRequest.netID});
    }
    else if (auto* type{std::get_if<EntityInteractionType>(
                 &castRequest.castableID)}) {
        result = world.castHelper.castEntityInteraction(
            {*type, clientEntity, castRequest.targetEntity,
             castRequest.targetPosition, castRequest.netID});
    }
    else if (auto* type{std::get_if<SpellType>(&castRequest.castableID)}) {
        result = world.castHelper.castSpell(
            {*type, clientEntity, castRequest.targetEntity,
             castRequest.targetPosition, castRequest.netID});
    }
    else {
        LOG_ERROR("Tried to cast Castable with invalid type.");
    }

    // If the cast failed, send the failure to the caster.
    if (result != CastFailureType::None) {
        network.serializeAndSend<CastFailed>(
            castRequest.netID,
            {clientEntity, castRequest.castableID, result});
    }
}

void CastSystem::updateCasts()
{
    auto view{world.registry.view<CastState>()};
    Uint32 currentTick{simulation.getCurrentTick()};

    // Iterate each entity that is currently casting.
    for (auto [entity, castState] : view.each()) {
        // If this cast is unstarted, start it.
        if (castState.endTick == 0) {
            startCast(castState);
            continue;
        }

        // If the entity has moved, cancel the cast.
        Position& position{world.registry.get<Position>(entity)};
        if (PreviousPosition* prevPosition{
                world.registry.try_get<PreviousPosition>(entity)}) {
            if (position != *prevPosition) {
                cancelCast(castState);
                continue;
            }
        }

        // If the cast has reached its finish time, finish it.
        if (currentTick == castState.endTick) {
            // Check that the cast is still valid, cancel it if not.
            if (!castIsValid(castState.castInfo, position)) {
                cancelCast(castState);
                continue;
            }

            finishCast(castState);
            continue;
        }
    }
}

void CastSystem::startCast(CastState& castState)
{
    // Note: CastHelper ensures that castState.castInfo.casterEntity is the 
    //       same entity that owns castState.

    // If this castable has any visuals, send a CastStarted to all nearby 
    // clients.
    // Note: If it has no visuals, there's no reason for a client to replicate
    //       it. Thus, we can skip sending the message.
    CastInfo& castInfo{castState.castInfo};
    if (castInfo.castable->hasVisuals()) {
        sendCastStarted(castState);
    }

    // If this cast triggers the GCD, track it.
    // Note: If CastCooldown gets created here, its lastUpdateTick will be
    //       set to currentTick.
    // Note: CastCooldown may get created here even if it isn't used. That's 
    //       fine, the entity has at least shown the capability to cast things.
    Uint32 currentTick{simulation.getCurrentTick()};
    CastCooldown& castCooldown{world.registry.get_or_emplace<CastCooldown>(
        castInfo.casterEntity, currentTick)};
    if (castInfo.castable->triggersGCD) {
        castCooldown.gcdTicksRemaining
            = SharedConfig::CAST_GLOBAL_COOLDOWN_TICKS;
    }

    // If this is an instant cast, finish it immediately.
    if (castInfo.castable->castTime == 0) {
        finishCast(castState);
    }
    else {
        // Not an instant cast. Set its end tick.
        Uint32 castTimeTicks{static_cast<Uint32>(
            castInfo.castable->castTime / SharedConfig::SIM_TICK_TIMESTEP_S)};
        castState.endTick = currentTick + castTimeTicks;
    }
}

void CastSystem::cancelCast(CastState& castState)
{
    // If this castable has any visuals, send a CastFailed to all 
    // nearby clients.
    // Note: If it has no visuals, there's no reason for a client 
    //       to replicate it. Thus, we can skip sending the message.
    CastInfo& castInfo{castState.castInfo};
    if (castInfo.castable->hasVisuals()) {
        sendCastFailed(castState, CastFailureType::Movement);
    }

    // Reset the GCD.
    CastCooldown& castCooldown{
        world.registry.get<CastCooldown>(castInfo.casterEntity)};
    castCooldown.gcdTicksRemaining = 0;

    // Cancel the cast.
    world.registry.erase<CastState>(castInfo.casterEntity);
}

void CastSystem::finishCast(CastState& castState)
{
    // If this castable has a cooldown, track it.
    const CastInfo& castInfo{castState.castInfo};
    if (castInfo.castable->cooldownTime > 0) {
        Uint32 castTimeTicks{
            static_cast<Uint32>(castInfo.castable->cooldownTime
                                / SharedConfig::SIM_TICK_TIMESTEP_S)};

        CastCooldown& castCooldown{
            world.registry.get<CastCooldown>(castInfo.casterEntity)};
        castCooldown.cooldowns.emplace_back(castInfo.castable->castableID,
                                            castTimeTicks);
    }

    // Handle the cast.
    handleCast(castState.castInfo);
    world.registry.erase<CastState>(castState.castInfo.casterEntity);
}

void CastSystem::sendCastStarted(CastState& castState)
{
    // Serialize a CastStarted message.
    CastInfo& castInfo{castState.castInfo};
    CastStarted castStarted{.casterEntity{castInfo.casterEntity},
                            .castableID{castInfo.castable->castableID},
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
    // Note: We skip the caster so that they don't restart a cast that they're
    //       already replicating.
    auto view{world.registry.view<Position, ClientSimData>()};
    for (entt::entity entity : *entitiesInRange) {
        if ((entity != castInfo.casterEntity) && view.contains(entity)) {
            const auto& client{view.get<ClientSimData>(entity)};
            network.send(client.netID, message);
        }
    }
}

void CastSystem::sendCastFailed(CastState& castState,
                                CastFailureType failureType)
{
    // Serialize a CastFailed message.
    CastInfo& castInfo{castState.castInfo};
    CastFailed castFailed{.casterEntity{castInfo.casterEntity},
                          .castableID{castInfo.castable->castableID},
                          .castFailureType{failureType}};
    BinaryBufferSharedPtr message{network.serialize(castFailed)};

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
    // Note: We skip the caster since clients predict all of the same types 
    //       of failure for their own player entity.
    auto view{world.registry.view<Position, ClientSimData>()};
    for (entt::entity entity : *entitiesInRange) {
        if ((entity != castInfo.casterEntity) && view.contains(entity)) {
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

bool CastSystem::castIsValid(const CastInfo& castInfo,
                             const Vector3& casterPosition)
{
    const Castable& castable{*(castInfo.castable)};
    Castable::TargetType targetType{castInfo.castable->targetType};
    entt::entity targetEntity{castInfo.targetEntity};
    if ((targetType == Castable::TargetType::Entity)
        || ((targetType == Castable::TargetType::SelfOrEntity)
            && (targetEntity != entt::null))) {
        // Check that the target entity is still alive.
        if (!(world.registry.valid(castInfo.targetEntity))) {
            return false;
        }

        // Check that the caster is in range of the target entity.
        const Position& targetPosition{
            world.registry.get<Position>(targetEntity)};
        float squaredRange{castable.range * castable.range};
        if (casterPosition.squaredDistanceTo(targetPosition) > squaredRange) {
            return false;
        }

        // Check that the target entity is in the caster's line of sight.
        if (!world.castHelper.isInLineOfSight(castInfo.casterEntity,
                                              targetEntity, casterPosition,
                                              targetPosition)) {
            return false;
        }
    }
    else if (targetType == Castable::TargetType::Circle) {
        // Check that the target position is within the map bounds.
        Cylinder targetCylinder{
            castable.getTargetCylinder(castInfo.targetPosition)};
        if (!(world.tileMap.getTileExtent().contains(targetCylinder))) {
            return false;
        }

        // Check that the caster is in range of the target position.
        float squaredDistance{
            casterPosition.squaredDistanceTo(castInfo.targetPosition)};
        if (squaredDistance > castable.range) {
            return false;
        }

        // Check that the target position is in the caster's line of sight.
        if (!world.castHelper.isInLineOfSight(castInfo.casterEntity,
                                              casterPosition,
                                              castInfo.targetPosition)) {
            return false;
        }
    }

    return true;
}

} // namespace Server
} // namespace AM
