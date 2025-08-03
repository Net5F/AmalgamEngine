#pragma once

#include "CastableID.h"
#include "Castable.h"
#include "ItemInteractionType.h"
#include "EntityInteractionType.h"
#include "SpellType.h"
#include <unordered_map>

namespace AM
{
class GraphicDataBase;

/**
 * Holds Castable data.
 * 
 * Castables are created at runtime by C++ code, but they're intended to be 
 * initialized once at construction time and immutable thereafter. The effect 
 * is the same as if they were being loaded from a shared file.
 */
class CastableData
{
public:
    CastableData(const GraphicDataBase& graphicData);

    /**
     * @return If no castable with the given ID exists, returns nullptr. Else,
     *         returns the requested castable.
     * Note: This returns nullptr for the null castable.
     */
    const Castable* getCastable(CastableID castableID) const;

private:
    /**
     * Adds the given castable to the appropriate map.
     */
    void addCastable(const Castable& castable);

    // Note: We use maps for these instead of arrays because the interaction 
    //       enums have big gaps (we reserve values since they get serialized 
    //       and would have to be migrated if we changed them). If there's 
    //       significant performance benefit in using an array, we can switch 
    //       and just live with the space usage.
    /** Holds item interactions, indexed by their ItemInteractionType. */
    std::unordered_map<ItemInteractionType, Castable> itemInteractionMap;

    /** Holds entity interaction, indexed by their EntityInteractionType. */
    std::unordered_map<EntityInteractionType, Castable> entityInteractionMap;

    /** Holds spells, indexed by their SpellType. */
    std::unordered_map<SpellType, Castable> spellMap;
};

} // namespace AM
