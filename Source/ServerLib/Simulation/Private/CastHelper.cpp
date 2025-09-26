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
#include "Collision.h"
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
    const Position& casterPosition{registry.get<Position>(casterEntity)};
    if (castable.castTime != 0) {
        // Note: Since we compare to the previous tick's position, it takes an 
        //       extra tick after movement stops before we let casts go through.
        //       We consider this to be fine.
        const PreviousPosition* previousPosition{
            registry.try_get<PreviousPosition>(casterEntity)};
        if (previousPosition && (casterPosition != *previousPosition)) {
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
        || (castable.targetType == Castable::TargetType::Entity)) {
        if (!(registry.valid(targetEntity))) {
            return CastFailureType::InvalidTargetEntity;
        }
    }

    // If the Castable uses a target entity, validate it.
    if ((castable.targetType == Castable::TargetType::Entity)
        || ((castable.targetType == Castable::TargetType::SelfOrEntity)
            && (targetEntity != entt::null))) {
        // Check that the caster is in range of the target entity.
        // Note: We already checked that both entities exist above.
        // Note: targetEntityPos is different than the targetPosition param.
        const Position& targetEntityPos{registry.get<Position>(targetEntity)};
        float squaredRange{castable.range * castable.range};
        if (casterPosition.squaredDistanceTo(targetEntityPos) > squaredRange) {
            return CastFailureType::OutOfRange;
        }

        // Check that the target entity is in the caster's line of sight.
        if (!isInLineOfSight(casterEntity, targetEntity, casterPosition,
                             targetPosition)) {
            return CastFailureType::LineOfSight;
        }
    }
    // If the Castable uses a target circle, validate it.
    else if (castable.targetType == Castable::TargetType::Circle) {
        // Check that the target position is within the map bounds.
        Cylinder targetCylinder{castable.getTargetCylinder(targetPosition)};
        if (!(world.tileMap.getTileExtent().contains(targetCylinder))) {
            return CastFailureType::InvalidTargetPosition;
        }

        // Check that the caster is in range of the target position.
        float squaredDistance{casterPosition.squaredDistanceTo(targetPosition)};
        if (squaredDistance > castable.range) {
            return CastFailureType::OutOfRange;
        }

        // Check that the target position is in the caster's line of sight.
        if (!isInLineOfSight(casterEntity, casterPosition, targetPosition)) {
            return CastFailureType::LineOfSight;
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

bool CastHelper::isInLineOfSight(entt::entity casterEntity,
                                 entt::entity targetEntity,
                                 const Vector3& casterPosition,
                                 const Vector3& targetPosition)
{
    // Find the start/end points for a raycast. If the entities have a 
    // Collision, use a point at the top of it to approximate their head.
    // If not, fall back to their Position.
    Vector3 casterPoint{casterPosition};
    if (const Collision
        * casterCollision{world.registry.try_get<Collision>(casterEntity)}) {
        casterPoint = casterCollision->worldBounds.getTopCenterPoint();
    }
    Vector3 targetPoint{targetPosition};
    if (const Collision
        * targetCollision{world.registry.try_get<Collision>(targetEntity)}) {
        targetPoint = targetCollision->worldBounds.getTopCenterPoint();
    }

    // If the ray hits anything besides the target, return false. If not, 
    // return true.
    std::array<entt::entity, 1> entitiesToExclude{targetEntity};
    bool objectWasHit{world.collisionLocator.raycastAny(
        {.start{casterPoint},
         .end{targetPoint},
         .collisionMask{CollisionLayerType::TerrainWall
                        | CollisionLayerType::BlockLoS},
         .entitiesToExclude{entitiesToExclude}})};

    return !objectWasHit;
}

bool CastHelper::isInLineOfSight(entt::entity casterEntity,
                                 const Vector3& casterPosition,
                                 const Vector3& targetPosition)
{
    // Find the start/end points for a raycast. If the entity has a 
    // Collision, use a point at the top of it to approximate their head.
    // If not, fall back to their Position.
    Vector3 casterPoint{casterPosition};
    if (const Collision
        * casterCollision{world.registry.try_get<Collision>(casterEntity)}) {
        casterPoint = casterCollision->worldBounds.getTopCenterPoint();
    }

    // If the ray hits anything, return false. If not, return true.
    bool objectWasHit{world.collisionLocator.raycastAny(
        {.start{casterPoint},
         .end{targetPosition},
         .collisionMask{CollisionLayerType::TerrainWall
                        | CollisionLayerType::BlockLoS}})};
    return !objectWasHit;
}

} // namespace Server
} // namespace AM
