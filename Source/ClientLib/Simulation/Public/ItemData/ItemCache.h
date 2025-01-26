#pragma once

#include "Item.h"
#include <vector>

namespace AM
{
namespace Client
{

/**
 * Used to save/load ItemCache.bin.
 *
 * Holds a list of items in a serializable form.
 */
struct ItemCache
{
    /** Used as a "we should never hit this" cap on the number of items in the 
        cache. */
    static constexpr std::size_t MAX_ITEMS{10000};

    struct ItemEntry {
        Item item{};
        ItemVersion version{};
    };

    std::vector<ItemEntry> items{};
};

template<typename S>
void serialize(S& serializer, ItemCache::ItemEntry& itemEntry)
{
    serializer.object(itemEntry.item);
    serializer.value2b(itemEntry.version);
}

template<typename S>
void serialize(S& serializer, ItemCache& itemCache)
{
    serializer.container(itemCache.items, ItemCache::MAX_ITEMS);
}

} // End namespace Client
} // End namespace AM
