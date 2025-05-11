#include "CastSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "GraphicData.h"
#include "ItemData.h"
#include "CastableData.h"
#include "ClientCastState.h"
#include "CastCooldown.h"
#include "Castable.h"
#include "AVEffects.h"
#include "EnttGroups.h"

namespace AM
{
namespace Client
{

CastSystem::CastSystem(Simulation& inSimulation, Network& inNetwork,
                       const GraphicData& inGraphicData,
                       const CastableData& inCastableData)
: simulation{inSimulation}
, world{simulation.getWorld()}
, network{inNetwork}
, graphicData{inGraphicData}
, castableData{inCastableData}
, castStartedQueue{network.getEventDispatcher()}
, castFailedQueue{network.getEventDispatcher()}
, castCooldownInitQueue{network.getEventDispatcher()}
, castFailed{castFailedSig}
{
}

void CastSystem::processCasts()
{
    // Process any waiting cast messages.
    CastStarted castStarted{};
    while (castStartedQueue.pop(castStarted)) {
        handleCastStarted(castStarted);
    }

    CastFailed castFailed{};
    while (castFailedQueue.pop(castFailed)) {
        handleCastFailed(castFailed);
    }

    CastCooldownInit castCooldownInit{};
    while (castCooldownInitQueue.pop(castCooldownInit)) {
        handleCastCooldownInit(castCooldownInit);
    }

    // Update ongoing casts.
    updateCasts();
}

void CastSystem::handleCastStarted(const CastStarted& castStarted)
{
    entt::registry& registry{world.registry};

    // If the given entity doesn't exist, we can't do anything.
    if (!(registry.valid(castStarted.casterEntity))) {
        return;
    }

    // If this entity has an ongoing cast, kill it.
    registry.remove<ClientCastState>(castStarted.casterEntity);

    // Start the cast.
    const Castable* castable{castableData.getCastable(castStarted.castableID)};
    CastInfo castInfo{castable, castStarted.casterEntity, nullptr,
                      castStarted.targetEntity, castStarted.targetPosition};
    registry.emplace<ClientCastState>(castStarted.casterEntity, castInfo,
                                      ClientCastState::State::Casting);
}

void CastSystem::handleCastFailed(const CastFailed& castFailed)
{
    // If the given info lines up with an existing cast, kill it.
    if (auto* castState{
            world.registry.try_get<ClientCastState>(castFailed.casterEntity)}) {
        if (castState->castInfo.castable->castableID == castFailed.castableID) {
            world.registry.erase<ClientCastState>(castFailed.casterEntity);
        }
    }

    // If this is for the player entity, signal it to the UI.
    if (castFailed.casterEntity == world.playerEntity) {
        castFailedSig.publish(castFailed);
    }
}

void CastSystem::handleCastCooldownInit(
    const CastCooldownInit& castCooldownInit)
{
    // Apply the given CastCooldown to the player entity.
    // Note: We only receive this message for the player entity, never any 
    //       other entity.
    // Note: We may be replacing a predicted cooldown by doing this, but the 
    //       server-given cooldown will be more accurate anyway.
    world.registry.emplace_or_replace<CastCooldown>(
        world.playerEntity, castCooldownInit.castCooldown);
}

void CastSystem::updateCasts()
{
    auto view{world.registry.view<ClientCastState>()};
    auto movementGroup{EnttGroups::getMovementGroup(world.registry)};
    Uint32 currentTick{simulation.getCurrentTick()};

    // Iterate each entity that is currently casting.
    for (auto [entity, castState] : view.each()) {
        // If this cast is unstarted, start it.
        if (castState.endTick == 0) {
            startCast(castState);
            continue;
        }

        // If the player entity has moved, cancel the cast.
        // Note: We don't predict cast failures on non-player entities.
        // Note: The player entity always has a PreviousPosition.
        if (entity == world.playerEntity) {
            auto [position, prevPosition]
                = movementGroup.get<Position, PreviousPosition>(entity);
            if (position != prevPosition) {
                world.registry.erase<ClientCastState>(entity);
                continue;
            }
        }

        // If the cast has reached its finish time, finish it.
        if ((castState.state == ClientCastState::State::Casting)
            && (currentTick == castState.endTick)) {
            finishCast(castState);
            continue;
        }

        // If the "cast complete" animation has finished, end the cast.
        if ((castState.state == ClientCastState::State::CastComplete)
            && (currentTick == castState.endTick)) {
            world.registry.erase<ClientCastState>(entity);
            continue;
        }
    }
}

void CastSystem::startCast(ClientCastState& castState)
{
    // Note: CastHelper ensures that castState.castInfo.casterEntity is the 
    //       same entity that owns castState.

    // If this cast triggers the GCD, track it.
    // Note: If CastCooldown gets created here, its lastUpdateTick will be set 
    //       to currentTick.
    // Note: CastCooldown may get created here even if it isn't used. That's 
    //       fine, the entity has at least shown the capability to cast things.
    CastInfo& castInfo{castState.castInfo};
    Uint32 currentTick{simulation.getCurrentTick()};
    CastCooldown& castCooldown{world.registry.get_or_emplace<CastCooldown>(
        castInfo.casterEntity, currentTick)};
    if (castInfo.castable->triggersGCD) {
        castCooldown.gcdTicksRemaining
            = SharedConfig::CAST_GLOBAL_COOLDOWN_TICKS;
    }

    // If this castable has a cooldown, track it.
    if (castInfo.castable->cooldownTime > 0) {
        Uint32 castTimeTicks{
            static_cast<Uint32>(castInfo.castable->cooldownTime
                                / SharedConfig::SIM_TICK_TIMESTEP_S)};
        castCooldown.cooldowns.emplace_back(castInfo.castable->castableID,
                                            castTimeTicks);
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

void CastSystem::finishCast(ClientCastState& castState)
{
    // If the caster has a valid graphic, mark the cast as complete and
    // play its audio/visual effects.
    if (Uint32 castCompleteEndTick{getCastCompleteEndTick(
            castState.castInfo.casterEntity, *(castState.castInfo.castable))}) {
        castState.state = ClientCastState::State::CastComplete;
        castState.endTick = castCompleteEndTick;
    }
    else
    {
        // No valid graphic, end the cast.
        world.registry.erase<ClientCastState>(castState.castInfo.casterEntity);
    }

    // Play any AV effects and create any AV entities.
    playAVEffects(castState.castInfo);
}

Uint32 CastSystem::getCastCompleteEndTick(entt::entity entity,
                                          const Castable& castable)
{
    // Check that the Castable has a "cast complete" graphic.
    if (castable.castingGraphicType == EntityGraphicType::NotSet) {
        return 0;
    }

    // Check that the entity has graphics enabled.
    const GraphicState* graphicState{
        world.registry.try_get<GraphicState>(entity)};
    if (!graphicState) {
        return 0;
    }

    // Check that the entity's graphic set has the Castable's "cast complete" 
    // graphic type.
    const EntityGraphicSet& graphicSet{
        graphicData.getEntityGraphicSet(graphicState->graphicSetID)};
    auto graphicArrIt{
        graphicSet.graphics.find(castable.castCompleteGraphicType)};
    if (graphicArrIt == graphicSet.graphics.end()) {
        return 0;
    }

    // Check that the graphic array has at least one graphic.
    // Note: We use the first non-null graphic from the array, which may not
    //       match the entity's current direction. We assume that all 
    //       directions of this graphic type have the same length, so it 
    //       shouldn't matter.
    const GraphicRef* graphic{nullptr};
    for (const GraphicRef& graphicRef : graphicArrIt->second) {
        if (graphicRef.getGraphicID()) {
            graphic = &graphicRef;
            break;
        }
    }
    if (!graphic) {
        return 0;
    }

    // Check that the graphic is an animation (we use the animation's length to
    // know how long to be in the "cast complete" state, so we can't support 
    // individual sprites).
    const auto* animation{
        std::get_if<std::reference_wrapper<const Animation>>(graphic)};
    if (!animation) {
        return 0;
    }

    Uint32 castCompleteEndTick{simulation.getCurrentTick()
                               + animation->get().getLengthTicks()};
    return castCompleteEndTick;
}

void CastSystem::playAVEffects(const CastInfo& castInfo)
{
    // If this castable has AV effects, add them to the caster.
    if (!(castInfo.castable->castCompleteVisualEffects.empty())) {
        AVEffects& avEffects{
            world.registry.get_or_emplace<AVEffects>(castInfo.casterEntity)};

        for (const VisualEffect& visualEffect :
             castInfo.castable->castCompleteVisualEffects) {
            avEffects.visualEffects.emplace_back(visualEffect);
        }
    }

    // If this castable spawns AV entities, add them to the local-only registry.
}

} // End namespace Client
} // End namespace AM
