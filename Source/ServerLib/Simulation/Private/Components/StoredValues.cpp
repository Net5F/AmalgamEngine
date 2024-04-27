#include "StoredValues.h"
#include "World.h"

namespace AM
{
namespace Server
{

bool StoredValues::storeValue(std::string_view stringID, Uint32 newValue,
                              World& world)
{
    // Try to get a numeric ID for the given string ID.
    EntityStoredValueID numericID{world.getEntityStoredValueID(stringID)};
    if (!numericID) {
        return false;
    }

    // If we're setting the value to 0, don't add it to the map (default values 
    // don't need to be stored).
    if (newValue == 0) {
        // If the value already exists, erase it.
        auto valueIt{valueMap.find(numericID)};
        if (valueIt != valueMap.end()) {
            valueMap.erase(valueIt);
        }

        return true;
    }

    valueMap[numericID] = newValue;
    return true;
}

Uint32 StoredValues::getStoredValue(std::string_view stringID, World& world)
{
    // Try to get the numeric ID for the given string ID.
    EntityStoredValueID numericID{world.getEntityStoredValueID(stringID)};
    if (!numericID) {
        return 0;
    }

    // If the value exists, return it.
    auto valueIt{valueMap.find(numericID)};
    if (valueIt != valueMap.end()) {
        return valueIt->second;
    }

    // Value doesn't exist. Return the default.
    return 0;
}

} // End namespace Server
} // End namespace AM
