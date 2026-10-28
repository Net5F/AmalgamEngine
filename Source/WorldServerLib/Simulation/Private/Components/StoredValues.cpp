#include "StoredValues.h"

namespace AM
{
namespace WorldServer
{

bool StoredValues::storeValue(std::string_view stringID, Uint32 newValue)
{
    auto valueIt{valueMap.find(stringID)};

    // If we're setting the value to 0, don't add it to the map (default values
    // don't need to be stored).
    if (newValue == 0) {
        // If the value already exists, erase it.
        if (valueIt != valueMap.end()) {
            valueMap.erase(valueIt);
        }

        return true;
    }

    // If the value already exists, overwrite it.
    if (valueIt != valueMap.end()) {
        valueIt->second = newValue;
        return true;
    }

    // Value doesn't exist. Check that we can add it.
    if ((stringID.size() > MAX_STRING_ID_LENGTH)
        || (valueMap.size() >= MAX_STORED_VALUES)) {
        return false;
    }

    valueMap.emplace(stringID, newValue);
    return true;
}

Uint32 StoredValues::getStoredValue(std::string_view stringID) const
{
    // If the value exists, return it.
    auto valueIt{valueMap.find(stringID)};
    if (valueIt != valueMap.end()) {
        return valueIt->second;
    }

    // Value doesn't exist. Return the default.
    return 0;
}

} // End namespace WorldServer
} // End namespace AM
