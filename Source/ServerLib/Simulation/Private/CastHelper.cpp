#include "CastHelper.h"
#include "Simulation.h"
#include "World.h"
#include "ItemData.h"
#include "CastableData.h"
#include "Inventory.h"
#include "Castable.h"
#include "CastState.h"
#include "ValidateCast.h"
#include "CastFailed.h"
#include "CastStarted.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "ClientSimData.h"
#include "CastCooldown.h"
#include "Cylinder.h"
#include "SharedConfig.h"

namespace AM
{
namespace Server
{

CastHelper::CastHelper(Simulation& inSimulation, const ItemData& inItemData,
                       const CastableData& inCastableData)
: onItemInteractionCompletedMap{}
, onEntityInteractionCompletedMap{}
, onSpellCompletedMap{}
, simulation{inSimulation}
, world{inSimulation.getWorld()}
, itemData{inItemData}
, castableData{inCastableData}
{
}

CastFailureType
    CastHelper::castItemInteraction(const CastItemInteractionParams& params)
{
    // Check that the item exists and actually has this interaction type.
    // Note: This implicitly checks that the entity owns the item, since it
    //       uses the slot index.
    // Note: If we ever hit a situation where the item in the requested slot
    //       doesn't match what the user clicked, we can also put the item ID
    //       in the request message to confirm.
    const auto& inventory{world.registry.get<Inventory>(params.casterEntity)};
    const Item* item{inventory.getItem(params.slotIndex, itemData)};
    if (!item) {
        return CastFailureType::InvalidItem;
    }
    else if (!(item->supportsInteraction(params.interactionType))) {
        return CastFailureType::InteractionNotSupported;
    }

    // Check that the requested interaction has an associated Castable.
    const Castable* castable{castableData.getCastable(params.interactionType)};
    if (!castable) {
        return CastFailureType::InvalidCastable;
    }

    // Perform the shared validation checks.
    CastFailureType failureType{
        performSharedChecks(*castable, params.casterEntity, params.targetEntity,
                            params.targetPosition)};
    if (failureType != CastFailureType::None) {
        return failureType;
    }

    // Add a CastState to the entity (we checked above that they aren't already 
    // casting).
    world.registry.emplace<CastState>(
        params.casterEntity,
        CastState{.castInfo{castable, params.casterEntity, item,
                            params.targetEntity, params.targetPosition,
                            params.clientID},
                  .endTick{0}});

    return CastFailureType::None;
}

CastFailureType
    CastHelper::castEntityInteraction(const CastEntityInteractionParams& params)
{
    // Check that the requested interaction has an associated Castable.
    const Castable* castable{castableData.getCastable(params.interactionType)};
    if (!castable) {
        return CastFailureType::InvalidCastable;
    }

    // Perform the shared validation checks.
    CastFailureType failureType{
        performSharedChecks(*castable, params.casterEntity, params.targetEntity,
                            params.targetPosition)};
    if (failureType != CastFailureType::None) {
        return failureType;
    }

    // We check above that the target entity is valid if one was provided, but 
    // entity interactions require a target entity. Check that it's non-null.
    if (params.targetEntity == entt::null) {
        return CastFailureType::InvalidTargetEntity;
    }

    // Add a CastState to the entity (we checked above that they aren't already 
    // casting).
    world.registry.emplace<CastState>(
        params.casterEntity,
        CastState{.castInfo{castable, params.casterEntity, nullptr,
                            params.targetEntity, params.targetPosition,
                            params.clientID},
                  .endTick{0}});

    return CastFailureType::None;
}

CastFailureType CastHelper::castSpell(const CastSpellParams& params)
{
    // Check that the requested interaction has an associated Castable.
    const Castable* castable{castableData.getCastable(params.interactionType)};
    if (!castable) {
        return CastFailureType::InvalidCastable;
    }

    // Perform the shared validation checks.
    CastFailureType failureType{
        performSharedChecks(*castable, params.casterEntity, params.targetEntity,
                            params.targetPosition)};
    if (failureType != CastFailureType::None) {
        return failureType;
    }

    // We check above that the target entity is valid if one was provided, but 
    // entity interactions require a target entity. Check that it's non-null.
    if (params.targetEntity == entt::null) {
        return CastFailureType::InvalidTargetEntity;
    }

    // Add a CastState to the entity (we checked above that they aren't already 
    // casting).
    world.registry.emplace<CastState>(
        params.casterEntity,
        CastState{.castInfo{castable, params.casterEntity, nullptr,
                            params.targetEntity, params.targetPosition,
                            params.clientID},
                  .endTick{0}});

    return CastFailureType::None;
}

void CastHelper::setOnItemInteractionCompleted(
    ItemInteractionType interactionType,
    std::function<void(const CastInfo&)> callback)
{
    onItemInteractionCompletedMap[interactionType] = callback;
}

void CastHelper::setOnEntityInteractionCompleted(
    EntityInteractionType interactionType,
    std::function<void(const CastInfo&)> callback)
{
    onEntityInteractionCompletedMap[interactionType] = callback;
}

void CastHelper::setOnSpellCompleted(
    SpellType spellType, std::function<void(const CastInfo&)> callback)
{
    onSpellCompletedMap[spellType] = callback;
}

CastFailureType CastHelper::performSharedChecks(const Castable& castable,
                                                entt::entity casterEntity,
                                                entt::entity targetEntity,
                                                const Vector3& targetPosition)
{
    entt::registry& registry{world.registry};

    // Check that the caster entity exists.
    // Note: This should only be able to fail if a non-CastSystem caller 
    //       doesn't fill the struct properly.
    if (!(registry.valid(casterEntity))) {
        return CastFailureType::InvalidCasterEntity;
    }

    // If this isn't an instant cast, check that the caster isn't moving.
    if (castable.castTime != 0) {
        // Note: Since we compare to the previous tick's position, it takes an 
        //       extra tick after movement stops before we let casts go through.
        //       We consider this to be fine.
        const Position& position{registry.get<Position>(casterEntity)};
        const PreviousPosition* previousPosition{
            registry.try_get<PreviousPosition>(casterEntity)};
        if (previousPosition && (position != *previousPosition)) {
            return CastFailureType::Movement;
        }
    }

    // Check that the caster isn't already casting.
    if (registry.all_of<CastState>(casterEntity)) {
        return CastFailureType::AlreadyCasting;
    }

    // Check that this cast isn't on cooldown, and the GCD isn't active.
    if (auto* castCooldown{registry.try_get<CastCooldown>(casterEntity)}) {
        if (castCooldown->isCastOnCooldown(castable.castableID,
                                           simulation.getCurrentTick())) {
            return CastFailureType::OnCooldown;
        }
    }

    // If a target entity was provided or the Castable requires a target entity,
    // check that it exists.
    if ((targetEntity != entt::null)
        || (castable.targetToolType == Castable::TargetToolType::Entity)) {
        if (!(registry.valid(targetEntity))) {
            return CastFailureType::InvalidTargetEntity;
        }
    }

    // If the Castable requires a target entity, check that the caster is 
    // in range of it.
    if (castable.targetToolType == Castable::TargetToolType::Entity) {
        // Check that the caster is in range of the target entity.
        // Note: We already checked that both entities exist above.
        const Position& casterPosition{registry.get<Position>(casterEntity)};
        const Position& targetPosition{registry.get<Position>(targetEntity)};
        float squaredRange{castable.range * castable.range};
        if (casterPosition.squaredDistanceTo(targetPosition) > squaredRange) {
            return CastFailureType::OutOfRange;
        }
    }

    // If the Castable requires a target circle, check that it's within the map
    // bounds and in range.
    if (castable.targetToolType == Castable::TargetToolType::Circle) {
        Cylinder targetCylinder{castable.getTargetCylinder(targetPosition)};
        if (!(world.tileMap.getTileExtent().contains(targetCylinder))) {
            return CastFailureType::InvalidTargetPosition;
        }

        const Position& casterEntityPos{registry.get<Position>(casterEntity)};
        float squaredDistance{
            casterEntityPos.squaredDistanceTo(targetPosition)};
        if (squaredDistance > castable.range) {
            return CastFailureType::OutOfRange;
        }
    }

    // Run the project's validation of the Castable's requirements.
    bool projectValidationSucceeded{
        validateCast(casterEntity, castable, registry)};
    if (!projectValidationSucceeded) {
        return CastFailureType::ProjectValidationFailed;
    }

    return CastFailureType::None;
};

} // namespace Server
} // namespace AM
