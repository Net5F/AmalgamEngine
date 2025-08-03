#pragma once

#include "CastInfo.h"
#include "CastFailureType.h"
#include "ItemInteractionType.h"
#include "EntityInteractionType.h"
#include "SpellType.h"
#include "NetworkID.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
#include <unordered_map>
#include <functional>

namespace AM
{
class CastableData;
struct Castable;
struct Vector3;

namespace Server
{
class Simulation;
class World;
class ItemData;

/**
 * Helper class for casting Castables. Also owns the maps that contain the 
 * cast handler callbacks.
 *
 * Note: "Helper" isn't the greatest name for this, since it also is the 
 *       only way to register handler callbacks. "world.castHelper" seems 
 *       like a pretty communicative, friendly interface though.
 */
class CastHelper
{
public:
    CastHelper(Simulation& inSimulation, const ItemData& inItemData,
               const CastableData& inCastableData);

    // Note: There's a potential failure where an entity stops moving, but a 
    //       system tries to cast before MovementSystem has ran, causing the 
    //       cast to still fail. We had to solve this on the client because the
    //       UI casts were failing, but it doesn't seem very likely to occur 
    //       here (any system that casts should be post-movement).
    struct CastItemInteractionParams;
    /**
     * Casts an item interaction, using the given info.
     * 
     * @return None if the cast is successful, otherwise returns the reason for
     *         failure.
     */
    CastFailureType
        castItemInteraction(const CastItemInteractionParams& params);

    struct CastEntityInteractionParams;
    /**
     * Casts an entity interaction, using the given info.
     * 
     * @return None if the cast is successful, otherwise returns the reason for
     *         failure.
     */
    CastFailureType
        castEntityInteraction(const CastEntityInteractionParams& params);

    struct CastSpellParams;
    /**
     * Casts a spell, using the given info.
     * 
     * @return None if the cast is successful, otherwise returns the reason for
     *         failure.
     */
    CastFailureType castSpell(const CastSpellParams& params);

    // Note: These functions are just to provide a friendly interface. The 
    //       maps are public, so you could just add to them directly.
    /**
     * Registers a callback for when an item interaction of the given type is 
     * successfully cast.
     * Note: Only one callback can be set for each type.
     */
    void setOnItemInteractionCompleted(
        ItemInteractionType interactionType,
        std::function<void(const CastInfo&)> callback);

    /**
     * Registers a callback for when an entity interaction of the given type is 
     * successfully cast.
     * Note: Only one callback can be set for each type.
     */
    void setOnEntityInteractionCompleted(
        EntityInteractionType interactionType,
        std::function<void(const CastInfo&)> callback);

    /**
     * Registers a callback for when a spell of the given type is successfully 
     * cast.
     * Note: Only one callback can be set for each type.
     */
    void setOnSpellCompleted(
        SpellType spellType,
        std::function<void(const CastInfo&)> callback);

    /**
     * Returns true if the target entity is in LoS of the caster entity.
     *
     * Tries to use each entity's Collision. If it doesn't have one, falls back
     * to its Position.
     *
     * @pre casterEntity and targetEntity must exist, casterPosition and 
     *      targetPosition must be valid positions within the world.
     */
    bool isInLineOfSight(entt::entity casterEntity, entt::entity targetEntity,
                         const Vector3& casterPosition,
                         const Vector3& targetPosition);

    /**
     * Overload for position-based target types.
     *
     * @pre casterEntity must exist, casterPosition and targetPosition must be
     *      valid positions within the world.
     */
    bool isInLineOfSight(entt::entity casterEntity,
                         const Vector3& casterPosition,
                         const Vector3& targetPosition);

    // Maps that hold handler callbacks for each type of cast. Only CastSystem 
    // should call these handlers, but we just set them as public to avoid 
    // overcomplication.
    std::unordered_map<ItemInteractionType,
                       std::function<void(const CastInfo&)>>
        onItemInteractionCompletedMap;
    std::unordered_map<EntityInteractionType,
                       std::function<void(const CastInfo&)>>
        onEntityInteractionCompletedMap;
    std::unordered_map<SpellType,
                       std::function<void(const CastInfo&)>>
        onSpellCompletedMap;

private:
    /**
     * Performs generic validation that's applicable to all 3 types of 
     * Castable.
     *
     * @return None if validation passed, otherwise returns the cause of 
     *         failure.
     */
    CastFailureType performSharedChecks(const Castable& castable,
                                        entt::entity casterEntity,
                                        entt::entity targetEntity,
                                        const Vector3& targetPosition);

    /** Used to get the current tick. */
    Simulation& simulation;
    World& world;
    const ItemData& itemData;
    const CastableData& castableData;

public:
struct CastItemInteractionParams {
    /** The item interaction to cast. */
    ItemInteractionType interactionType{};
    /** The entity that is casting. */
    entt::entity casterEntity{};
    /** The slot of the item that is being used, within casterEntity's 
        inventory. */
    Uint8 slotIndex{};
    /** (Optional) The target entity. If the Castable's targetToolType is 
        Entity, this must be valid. */
    entt::entity targetEntity{entt::null};
    /** (Optional) The target position. If the Castable's targetToolType 
        is Circle, this must be valid. */
    Vector3 targetPosition{std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max()};
    /** (Optional) The client that requested this cast. If casterEntity is a 
        client entity, you must fill this in. Otherwise it won't be replicated 
        properly. */
    NetworkID clientID{NULL_NETWORK_ID};
};

struct CastEntityInteractionParams {
    /** The entity interaction to cast. */
    EntityInteractionType interactionType{};
    /** The entity that is casting. */
    entt::entity casterEntity{entt::null};
    /** The target entity. */
    entt::entity targetEntity{entt::null};
    /** (Optional) The target position. If the Castable's targetToolType 
        is Circle, this must be valid. */
    Vector3 targetPosition{std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max()};
    /** (Optional) The client that requested this cast. If casterEntity is a 
        client entity, you must fill this in. Otherwise it won't be replicated 
        properly. */
    NetworkID clientID{NULL_NETWORK_ID};
};

struct CastSpellParams {
    /** The item interaction to cast. */
    SpellType interactionType{};
    /** The entity that is casting. */
    entt::entity casterEntity{entt::null};
    /** (Optional) The target entity. If the Castable's targetToolType is 
        Entity, this must be valid. */
    entt::entity targetEntity{entt::null};
    /** (Optional) The target position. If the Castable's targetToolType 
        is Circle, this must be valid. */
    Vector3 targetPosition{std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max()};
    /** (Optional) The client that requested this cast. If casterEntity is a 
        client entity, you must fill this in. Otherwise it won't be replicated 
        properly. */
    NetworkID clientID{NULL_NETWORK_ID};
};
};

} // End namespace Server
} // End namespace AM
