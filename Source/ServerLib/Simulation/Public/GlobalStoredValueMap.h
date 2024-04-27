#pragma once

#include "HashTools.h"
#include "bitsery/ext/std_map.h"
#include <SDL_stdinc.h>
#include <unordered_map>
#include <string>

namespace AM
{

/**
 * A global key-value store.
 * Note: Entity stored values are handled separately. This file only applies 
 *       to values stored in World::globalStoredValueMap.
 *
 * Global stored values can't re-use string IDs, regardless of type. However, an 
 * entity stored value can have the same string ID as a global stored value.
 *
 * string_hash and equal_to are added for heterogeneous access (lets us use 
 * string_views directly from Lua without allocating an intermediate 
 * std::string).
 */
using GlobalStoredValueMap
    = std::unordered_map<std::string, Uint32, string_hash, std::equal_to<>>;

/** Used as a "we should never hit this" cap on the number of entity stored 
    values. */
static constexpr std::size_t MAX_GLOBAL_STORED_VALUES{SDL_MAX_UINT16};

/** Used as a "we should never hit this" cap on stored value string ID length. */
static constexpr std::size_t MAX_GLOBAL_STORED_VALUE_STRING_ID_LENGTH{500};

template<typename S>
void serialize(S& serializer, GlobalStoredValueMap& globalStoredValueMap)
{
    serializer.ext(globalStoredValueMap,
                   bitsery::ext::StdMap{MAX_GLOBAL_STORED_VALUES},
                   [](S& serializer, std::string& stringID, Uint32& value) {
                       serializer.text1b(
                           stringID, MAX_GLOBAL_STORED_VALUE_STRING_ID_LENGTH);
                       serializer.value4b(value);
                   });
}

} // End namespace AM
