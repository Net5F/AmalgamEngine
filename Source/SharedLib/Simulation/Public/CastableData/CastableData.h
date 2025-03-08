#pragma once

#include "ItemInteraction.h"
#include "EntityInteraction.h"
#include "Spell.h"
#include "ItemInteractionType.h"
#include "EntityInteractionType.h"
#include "SpellType.h"
#include <unordered_map>

namespace AM
{

class CastableData
{
public:
    CastableData();

    const ItemInteraction& getItemInteraction(ItemInteractionType type) const;

    const EntityInteraction&
        getEntityInteraction(EntityInteractionType type) const;

    const Spell& getSpell(SpellType type) const;

private:
    void addItemInteraction(ItemInteractionType type,
                            const ItemInteraction& itemInteraction);
    void addEntityInteraction(EntityInteractionType type,
                              const EntityInteraction& entityInteraction);
    void addSpell(SpellType type, const Spell& spell);

    // Note: We use maps for these instead of arrays because the interaction 
    //       enums have big gaps (we reserve values since they get serialized 
    //       and would have to be migrated if we changed them). If there's 
    //       significant performance benefit in using an array, we can switch 
    //       and just live with the space usage.
    /** Holds item interactions, indexed by their ItemInteractionType. */
    std::unordered_map<ItemInteractionType, ItemInteraction> itemInteractionMap;

    /** Holds entity interaction, indexed by their EntityInteractionType. */
    std::unordered_map<EntityInteractionType, EntityInteraction>
        entityInteractionMap;

    /** Holds spells, indexed by their SpellType. */
    std::unordered_map<SpellType, Spell> spellMap;
};

} // namespace AM
