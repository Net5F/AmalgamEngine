#pragma once

#include "ItemID.h"
#include <string>

namespace AM
{

/**
 * Defines a combination between two items.
 *
 * Occurs when a user "Use"s an item, then selects a target item.
 */
struct ItemCombination {
    /** The max length of the description text. */
    static constexpr std::size_t MAX_DESCRIPTION_LENGTH{500};

    /** The item to combine with. */
    ItemID otherItemID{NULL_ITEM_ID};

    /** The resulting item. */
    ItemID resultItemID{NULL_ITEM_ID};

    /** The descriptive text to send to the client when this combination
        succeeds. */
    std::string description{};
};

template<typename S>
void serialize(S& serializer, ItemCombination& itemCombination)
{
    serializer.value2b(itemCombination.otherItemID);
    serializer.value2b(itemCombination.resultItemID);
    serializer.text1b(itemCombination.description,
                      ItemCombination::MAX_DESCRIPTION_LENGTH);
}

} // End namespace AM
