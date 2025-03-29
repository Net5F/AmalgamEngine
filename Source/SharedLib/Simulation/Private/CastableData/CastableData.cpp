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
    // Init all of the project's Castables.
    CastableInit::initCastables(
        [this](CastableID castableID, const Castable& castable) {
            addCastable(castableID, castable);
        });

    // Note: We intentionally don't have a "null castable", because we want the 
    //       getters to return nullptr if there isn't a real castable to return.
}

const Castable* CastableData::getCastable(CastableID castableID) const
{
    // Attempt to find the Castable.
    if (auto* type{std::get_if<ItemInteractionType>(&castableID)}) {
        auto it{itemInteractionMap.find(*type)};
        if (it != itemInteractionMap.end()) {
            return &(it->second);
        }
    }
    else if (auto* type{std::get_if<EntityInteractionType>(&castableID)}) {
        auto it{entityInteractionMap.find(*type)};
        if (it != entityInteractionMap.end()) {
            return &(it->second);
        }
    }
    else if (auto* type{std::get_if<SpellType>(&castableID)}) {
        auto it{spellMap.find(*type)};
        if (it != spellMap.end()) {
            return &(it->second);
        }
    }
    else {
        LOG_ERROR("Tried to get Castable with invalid type.");
    }

    // Failed to find the requested castable.
    return nullptr;
}

void CastableData::addCastable(CastableID castableID, const Castable& castable)
{
    if (auto* type{std::get_if<ItemInteractionType>(&castableID)}) {
        itemInteractionMap[*type] = castable;
    }
    else if (auto* type{std::get_if<EntityInteractionType>(&castableID)}) {
        entityInteractionMap[*type] = castable;
    }
    else if (auto* type{std::get_if<SpellType>(&castableID)}) {
        spellMap[*type] = castable;
    }
    else {
        LOG_ERROR("Tried to add Castable with invalid type.");
    }
}

} // End namespace AM
