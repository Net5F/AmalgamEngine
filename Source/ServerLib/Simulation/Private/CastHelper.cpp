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

//void CastSystem::processEntityInteractionCastRequest(
//    const CastRequest& castRequest)
//{
    //// If there's a waiting message of the given type, validate it.
    //std::queue<EntityInteractionRequest>& queue{
    //    entityInteractionQueueMap[interactionType]};
    //if (!(queue.empty())) {
    //    EntityInteractionRequest interactionRequest{queue.front()};
    //    queue.pop();
    //    entt::registry& registry{world.registry};

    //    // Find the client's entity ID.
    //    auto it{world.netIDMap.find(interactionRequest.netID)};
    //    if (it == world.netIDMap.end()) {
    //        // Client doesn't exist (may have disconnected), do nothing.
    //        return false;
    //    }
    //    entt::entity clientEntity{it->second};
    //    entt::entity targetEntity{interactionRequest.targetEntity};

    //    // Check that the target exists.
    //    if (!(registry.valid(targetEntity))) {
    //        return false;
    //    }

    //    // Check that the client is in range of the target.
    //    const Position& clientPosition{registry.get<Position>(clientEntity)};
    //    const Position& targetPosition{registry.get<Position>(targetEntity)};
    //    if (clientPosition.squaredDistanceTo(targetPosition)
    //        > SharedConfig::SQUARED_INTERACTION_DISTANCE) {
    //        network.serializeAndSend(
    //            interactionRequest.netID,
    //            SystemMessage{"You must move closer to interact with that."});
    //        return false;
    //    }

    //    // Check that the target actually has this interaction type.
    //    if (auto* interaction{registry.try_get<Interaction>(targetEntity)};
    //        !interaction
    //        || !(interaction->supports(interactionRequest.interactionType))) {
    //        return false;
    //    }

    //    // Request is valid. Return it.
    //    data.clientEntity = clientEntity;
    //    data.targetEntity = targetEntity;
    //    data.clientID = interactionRequest.netID;
    //    return true;
    //}

    //return false;
//}

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
    // Check that the caster isn't already casting.
    if (world.registry.all_of<CastState>(casterEntity)) {
        return CastFailureType::AlreadyCasting;
    }

    // If the Castable requires a target entity, check that it exists.
    if ((castable.targetToolType == Castable::TargetToolType::Entity)
        && !(world.registry.valid(targetEntity))) {
        return CastFailureType::InvalidTargetEntity;
    }

    // If the Castable requires a target position, check that it's in range.
    if (castable.targetToolType == Castable::TargetToolType::Circle) {
        const Position& casterEntityPos{
            world.registry.get<Position>(casterEntity)};
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
        validateCast(casterEntity, castable, world.registry)};
    if (!projectValidationSucceeded) {
        return CastFailureType::ProjectValidationFailed;
    }

    return CastFailureType::None;
};

} // namespace Server
} // namespace AM
