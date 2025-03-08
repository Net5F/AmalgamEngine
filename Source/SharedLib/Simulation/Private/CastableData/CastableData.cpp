#include "CastableData.h"
#include "CastableInit.h"
#include "Log.h"

namespace AM
{

CastableData::CastableData()
: itemInteractionMap{}
, entityInteractionMap{}
, spellMap{}
{
    // Add an empty struct, to use as a default return.
    itemInteractionMap.emplace(ItemInteractionType::NotSet, ItemInteraction{});
    entityInteractionMap.emplace(EntityInteractionType::NotSet,
                                 EntityInteraction{});
    spellMap.emplace(SpellType::NotSet, Spell{});

    // Init each type of castable.
    CastableInit::initItemInteractions(
        [this](ItemInteractionType type,
               const ItemInteraction& itemInteraction) {
            addItemInteraction(type, itemInteraction);
        });
    CastableInit::initEntityInteractions(
        [this](EntityInteractionType type,
               const EntityInteraction& itemInteraction) {
            addEntityInteraction(type, itemInteraction);
        });
    CastableInit::initSpells(
        [this](SpellType type, const Spell& itemInteraction) {
            addSpell(type, itemInteraction);
        });
}

const ItemInteraction&
    CastableData::getItemInteraction(ItemInteractionType type) const
{
    // Attempt to find the interaction.
    auto it{itemInteractionMap.find(type)};
    if (it == itemInteractionMap.end()) {
        LOG_ERROR("Tried to get invalid item interaction.");
        return itemInteractionMap.at(ItemInteractionType::NotSet);
    }

    return it->second;
}

const EntityInteraction&
    CastableData::getEntityInteraction(EntityInteractionType type) const
{
    // Attempt to find the interaction.
    auto it{entityInteractionMap.find(type)};
    if (it == entityInteractionMap.end()) {
        LOG_ERROR("Tried to get invalid entity interaction.");
        return entityInteractionMap.at(EntityInteractionType::NotSet);
    }

    return it->second;
}

const Spell& CastableData::getSpell(SpellType type) const
{
    // Attempt to find the spell.
    auto it{spellMap.find(type)};
    if (it == spellMap.end()) {
        LOG_ERROR("Tried to get invalid spell.");
        return spellMap.at(SpellType::NotSet);
    }

    return it->second;
}

void CastableData::addItemInteraction(ItemInteractionType type,
                                      const ItemInteraction& itemInteraction)
{
    itemInteractionMap[type] = itemInteraction;
}

void CastableData::addEntityInteraction(EntityInteractionType type,
                                        const EntityInteraction& entityInteraction)
{
    entityInteractionMap[type] = entityInteraction;
}

void CastableData::addSpell(SpellType type, const Spell& spell)
{
    spellMap[type] = spell;
}

} // End namespace AM
