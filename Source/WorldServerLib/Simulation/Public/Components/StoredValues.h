#pragma once

#include "HashTools.h"
#include "bitsery/ext/std_map.h"
#include <SDL3/SDL_stdinc.h>
#include <unordered_map>
#include <string>
#include <string_view>

namespace AM
{
namespace WorldServer
{
/**
 * A per-entity key-value store.
 *
 * Values are identified using their string ID.
 *
 * Global values are also supported (see World::globalStoredValueMap).
 */
struct StoredValues {
    /** Used as a "we should never hit this" cap on the number of values that
        component can hold. */
    static constexpr std::size_t MAX_STORED_VALUES{SDL_MAX_UINT16};

    /** The max length of a stored value's string ID. */
    static constexpr std::size_t MAX_STRING_ID_LENGTH{20};

    /**
     * Holds the entity's stored values.
     *
     * Maps string ID -> value.
     *
     * string_hash and equal_to are added for heterogeneous access (lets us use
     * string_views directly from Lua without allocating an intermediate
     * std::string).
     */
    std::unordered_map<std::string, Uint32, string_hash, std::equal_to<>>
        valueMap{};

    /**
     * Adds a new value, or overwrites an existing value.
     *
     * If newValue == 0 (the default value), the value will be deleted.
     *
     * Note: Stored values are often cast to different types, but their
     *       underlying type is always Uint32.
     *
     * @param stringID The string ID of the value to add or overwrite. Must
     *                 already be derived (see StringTools::deriveStringID()).
     * @param newValue The new value to use.
     * @return True if the value was successfully set, else false (string ID is
     *         too long, or value with the given ID doesn't exist and value
     *         limit is reached).
     */
    bool storeValue(std::string_view stringID, Uint32 newValue);

    /**
     * Gets a stored value.
     *
     * Note: Stored values are often cast to different types, but their
     *       underlying type is always Uint32.
     *
     * @param stringID The string ID of the value to get. Must already be
     *                 derived (see StringTools::deriveStringID()).
     * @return The requested value. If not found, returns 0 (the default value
     *         that the value would have if it existed).
     */
    Uint32 getStoredValue(std::string_view stringID) const;
};

template<typename S>
void serialize(S& serializer, StoredValues& storedValues)
{
    serializer.ext(
        storedValues.valueMap,
        bitsery::ext::StdMap{StoredValues::MAX_STORED_VALUES},
        [](S& serializer, std::string& stringID, Uint32& value) {
            serializer.text1b(stringID, StoredValues::MAX_STRING_ID_LENGTH);
            serializer.value4b(value);
        });
}

} // namespace WorldServer
} // namespace AM
