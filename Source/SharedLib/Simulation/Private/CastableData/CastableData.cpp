#include "CastableData.h"
#include "GraphicDataBase.h"
#include "EngineCastableDef.h"
#include "CastableDef.h"
#include "Log.h"

namespace AM
{

CastableData::CastableData(const GraphicDataBase& graphicData)
: itemInteractionMap{}
, entityInteractionMap{}
, spellMap{}
{
    // Init all of the engine's Castables.
    EngineCastableDef::defineCastables(graphicData, 
        [&](CastableID castableID, const Castable& castable) {
            addCastable(castableID, castable);
        });

    // Init all of the project's Castables.
    CastableDef::defineCastables(graphicData,
        [&](CastableID castableID, const Castable& castable) {
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
    // Create the new castable (will overwrite an existing castable, if present).
    Castable* newCastable{};
    if (auto* type{std::get_if<ItemInteractionType>(&castableID)}) {
        newCastable = &(itemInteractionMap[*type]);
    }
    else if (auto* type{std::get_if<EntityInteractionType>(&castableID)}) {
        newCastable = &(entityInteractionMap[*type]);
    }
    else if (auto* type{std::get_if<SpellType>(&castableID)}) {
        newCastable = &(spellMap[*type]);
    }
    else {
        LOG_ERROR("Tried to add Castable with invalid type.");
        return;
    }

    // Copy the given castable and make sure it's set to use the given ID.
    *newCastable = castable;
    newCastable->castableID = castableID;
}

} // End namespace AM
