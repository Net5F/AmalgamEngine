#include "CastSystem.h"
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
#include "ClientSimData.h"
#include "Cylinder.h"
#include "SharedConfig.h"

namespace AM
{
namespace Server
{

CastHelper::CastHelper(World& inWorld, const ItemData& inItemData,
                       const CastableData& inCastableData)
: world{inWorld}
, itemData{inItemData}
, castableData{inCastableData}
, onItemInteractionCompletedMap{}
, onEntityInteractionCompletedMap{}
, onSpellCompletedMap{}
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
    // Note: This should only be able to happen if a non-CastSystem caller 
    //       doesn't fill the struct properly.
    if (registry.valid(targetEntity)) {
        return CastFailureType::InvalidCasterEntity;
    }

    // Check that the caster isn't already casting.
    if (registry.all_of<CastState>(casterEntity)) {
        return CastFailureType::AlreadyCasting;
    }

    // If a target entity was provided or the Castable requires a target entity.
    if ((targetEntity != entt::null)
        || (castable.targetToolType == Castable::TargetToolType::Entity)) {
        // Check that the target entity exists.
        if (registry.valid(targetEntity)) {
            return CastFailureType::InvalidTargetEntity;
        }

        // Check that the caster is in range of the target entity.
        const Position& casterPosition{registry.get<Position>(casterEntity)};
        const Position& targetPosition{registry.get<Position>(targetEntity)};
        float squaredRange{castable.range * castable.range};
        if (casterPosition.squaredDistanceTo(targetPosition) > squaredRange) {
            return CastFailureType::OutOfRange;
        }
    }

    // If the Castable requires a target position, check that it's in range.
    if (castable.targetToolType == Castable::TargetToolType::Circle) {
        const Position& casterEntityPos{registry.get<Position>(casterEntity)};
        if (!(world.tileMap.getTileExtent().contains(casterEntityPos))) {
            return CastFailureType::InvalidTargetPosition;
        }

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
