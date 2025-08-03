#pragma once

#include "CastInfo.h"
#include "CastFailureType.h"
#include "ItemInteractionType.h"
#include "EntityInteractionType.h"
#include "SpellType.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
#include <unordered_map>
#include <functional>
#include <limits>
#include <queue>

namespace AM
{
class CastableData;
struct Castable;
struct Vector3;

namespace Client
{
class Simulation;
class World;
class Network;
class ItemData;

/**
 * Helper class for casting Castables.
 */
class CastHelper
{
public:
    CastHelper(Simulation& inSimulation, Network& inNetwork,
               const ItemData& inItemData, const CastableData& inCastableData);

    struct CastItemInteractionParams;
    /**
     * Queues the given item interaction to be cast during the next CastSystem 
     * tick.
     */
    void queueItemInteraction(const CastItemInteractionParams& params);

    struct CastEntityInteractionParams;
    /**
     * Queues the given entity interaction to be cast during the next CastSystem
     * tick.
     */
    void queueEntityInteraction(const CastEntityInteractionParams& params);

    struct CastSpellParams;
    /**
     * Queues the given spell to be cast during the next CastSystem tick.
     */
    void queueSpell(const CastSpellParams& params);

private:
    /**
     * Casts an item interaction, using the given info.
     * 
     * @return None if the cast is successful, otherwise returns the reason for
     *         failure.
     */
    CastFailureType
        castItemInteraction(const CastItemInteractionParams& params);

    /**
     * Casts an entity interaction, using the given info.
     *
     * @return None if the cast is successful, otherwise returns the reason for
     *         failure.
     */
    CastFailureType
        castEntityInteraction(const CastEntityInteractionParams& params);

    /**
     * Casts a spell, using the given info.
     *
     * @return None if the cast is successful, otherwise returns the reason for
     *         failure.
     */
    CastFailureType castSpell(const CastSpellParams& params);

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
    /** Used to send cast requests. We could send them in CastSystem, but we 
        do it here to reduce latency as much as possible. */
    Network& network;
    const ItemData& itemData;
    const CastableData& castableData;

    // We friend CastSystem so it can use the queues and cast() functions 
    // without letting users get confused and call e.g. castSpell() instead of 
    // queueSpell(). Ideally, we would set this up differently to avoid 
    // exposing the rest of our private state.
    friend class CastSystem;

public:
struct CastItemInteractionParams {
    // Note: The player entity is always assumed to be the caster.

    /** The item interaction to cast. */
    ItemInteractionType interactionType{};
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
};

struct CastEntityInteractionParams {
    // Note: The player entity is always assumed to be the caster.

    /** The entity interaction to cast. */
    EntityInteractionType interactionType{};
    /** The target entity. */
    entt::entity targetEntity{entt::null};
    /** (Optional) The target position. If the Castable's targetToolType 
        is Circle, this must be valid. */
    Vector3 targetPosition{std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max()};
};

struct CastSpellParams {
    // Note: The player entity is always assumed to be the caster.

    /** The item interaction to cast. */
    SpellType interactionType{};
    /** (Optional) The target entity. If the Castable's targetToolType is 
        Entity, this must be valid. */
    entt::entity targetEntity{entt::null};
    /** (Optional) The target position. If the Castable's targetToolType 
        is Circle, this must be valid. */
    Vector3 targetPosition{std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max()};
};

private:
    // Queues that hold cast requests sent by the UI. 
    std::queue<CastItemInteractionParams> castItemInteractionQueue;
    std::queue<CastEntityInteractionParams> castEntityInteractionQueue;
    std::queue<CastSpellParams> castSpellQueue;
};

} // End namespace Client 
} // End namespace AM
