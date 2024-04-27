#pragma once

#include "EntityStoredValueID.h"
#include "bitsery/ext/std_map.h"
#include <unordered_map>
#include <string_view>

namespace AM
{
namespace Server
{
class World;

/**
 * A per-entity key-value store.
 * 
 * Values are identified in scripts using their string ID (see World::
 * storedValueIDMap). We map string ID -> numeric ID for storage, to save 
 * space and avoid allocations.
 * 
 * Global values are also supported (see World::globalStoredValueMap).
 */
struct StoredValues {
    /** Used as a "we should never hit this" cap on the number of values that 
        component can hold. */
    static constexpr std::size_t MAX_STORED_VALUES{SDL_MAX_UINT16};

    /**
     * Holds the entity's stored values.
     *
     * Maps numeric ID -> value.
     */
    std::unordered_map<EntityStoredValueID, Uint32> valueMap{};

    /**
     * Adds a new value, or overwrites an existing value.
     *
     * If newValue == 0 (the default value), the value will be deleted.
     *
     * Note: Stored values are often cast to different types, but their 
     *       underlying type is always Uint32.
     *
     * @param stringID The string ID of the value to add or overwrite.
     * @param newValue The new value to use.
     * @return True if the value was successfully set, else false (value with 
     *         the given ID doesn't exist and value limit is reached).
     */
    bool storeValue(std::string_view stringID, Uint32 newValue, World& world);

    /**
     * Gets a stored value.
     *
     * Note: Stored values are often cast to different types, but their 
     *       underlying type is always Uint32.
     * 
     * @param stringID The string ID of the value to get.
     * @return The requested value. If not found, returns 0 (the default value 
     *         that the value would have if it existed).
     */
    Uint32 getStoredValue(std::string_view stringID, World& world);
};

template<typename S>
void serialize(S& serializer, StoredValues& storedValues)
{
    serializer.ext(
        storedValues.valueMap,
        bitsery::ext::StdMap{StoredValues::MAX_STORED_VALUES},
        [](S& serializer, EntityStoredValueID& numericID, Uint32& value) {
            serializer.value2b(numericID);
            serializer.value4b(value);
        });
}

} // namespace Server
} // namespace AM
