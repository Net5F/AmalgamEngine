#pragma once

#include "CastInfo.h"
#include "CastFailureType.h"
#include "CastHelperParams.h"
#include <unordered_map>
#include <functional>

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

    /** Holds the handler callbacks for each type of item interaction. */
    std::unordered_map<ItemInteractionType,
                       std::function<void(const CastInfo&)>>
        onItemInteractionCompletedMap;

    /** Holds the handler callbacks for each type of entity interaction. */
    std::unordered_map<EntityInteractionType,
                       std::function<void(const CastInfo&)>>
        onEntityInteractionCompletedMap;

    /** Holds the handler callbacks for each type of spell. */
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
    /** Used to send cast requests. We could send them in CastSystem, but we 
        do it here to reduce latency as much as possible. */
    Network& network;
    const ItemData& itemData;
    const CastableData& castableData;
};

} // End namespace Client 
} // End namespace AM
