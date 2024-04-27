#pragma once

#include "EntityStoredValueID.h"
#include "HashTools.h"
#include "bitsery/ext/std_map.h"
#include <unordered_map>
#include <string>

namespace AM
{

/**
 * Maps entity stored value string IDs -> their associated numeric ID.
 * Note: Global stored values are handled separately. This file only applies 
 *       to values stored in an entity's StoredValues component.
 *
 * Entity stored values can't re-use string IDs, regardless of type. However, an 
 * entity stored value can have the same string ID as a global stored value.
 *
 * string_hash and equal_to are added for heterogeneous access (lets us use 
 * string_views directly from Lua without allocating an intermediate 
 * std::string).
 *
 * Note: One could imagine using a StoredValueInfo struct that contained both ID 
 *       and type, to introduce type safety. This ends up being fairly messy 
 *       though, and it doesn't seem to provide much value.
 */
using EntityStoredValueIDMap
    = std::unordered_map<std::string, EntityStoredValueID, string_hash,
                         std::equal_to<>>;

/** Used as a "we should never hit this" cap on the number of entity stored 
    values. */
static constexpr std::size_t MAX_ENTITY_STORED_VALUE_IDS{SDL_MAX_UINT16};

/** Used as a "we should never hit this" cap on stored value string ID length. */
static constexpr std::size_t MAX_ENTITY_STORED_VALUE_STRING_ID_LENGTH{500};

template<typename S>
void serialize(S& serializer, EntityStoredValueIDMap& entityStoredValueIDMap)
{
    serializer.ext(
        entityStoredValueIDMap,
        bitsery::ext::StdMap{MAX_ENTITY_STORED_VALUE_IDS},
        [](S& serializer, std::string& stringID, EntityStoredValueID& id) {
            serializer.text1b(stringID,
                              MAX_ENTITY_STORED_VALUE_STRING_ID_LENGTH);
            serializer.value2b(id);
        });
}

} // End namespace AM
