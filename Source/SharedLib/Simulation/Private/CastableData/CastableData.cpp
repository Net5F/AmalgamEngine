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
    EngineCastableDef::defineCastables(
        graphicData, [&](const Castable& castable) { addCastable(castable); });

    // Init all of the project's Castables.
    CastableDef::defineCastables(
        graphicData, [&](const Castable& castable) { addCastable(castable); });

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

void CastableData::addCastable(const Castable& castable)
{
    if (auto* type{std::get_if<ItemInteractionType>(&(castable.castableID))}) {
        if (*type == ItemInteractionType::NotSet) {
            LOG_FATAL("Tried to create castable with invalid ID.");
            return;
        }
        else if (itemInteractionMap.contains(*type)) {
            LOG_FATAL("Tried to create item interaction with ID that's already "
                      "in use: %u",
                      *type);
            return;
        }

        itemInteractionMap[*type] = castable;
    }
    else if (auto* type{
                 std::get_if<EntityInteractionType>(&(castable.castableID))}) {
        if (*type == EntityInteractionType::NotSet) {
            LOG_FATAL("Tried to create castable with invalid ID.");
            return;
        }
        else if (entityInteractionMap.contains(*type)) {
            LOG_FATAL("Tried to create entity interaction with ID that's "
                      "already in use: %u",
                      *type);
            return;
        }

        entityInteractionMap[*type] = castable;
    }
    else if (auto* type{std::get_if<SpellType>(&(castable.castableID))}) {
        if (*type == SpellType::NotSet) {
            LOG_FATAL("Tried to create castable with invalid ID.");
            return;
        }
        else if (spellMap.contains(*type)) {
            LOG_FATAL("Tried to create spell with ID that's already in use: %u",
                      *type);
            return;
        }

        spellMap[*type] = castable;
    }
    else {
        LOG_ERROR("Tried to add Castable with invalid type.");
        return;
    }
}

} // End namespace AM
