#include "ItemDataBase.h"
#include "ItemID.h"
#include "Log.h"
#include "AMAssert.h"
#include <algorithm>

namespace AM
{
ItemDataBase::ItemDataBase(bool inTrackItemUpdates)
: items{}
, itemVersions{}
, itemStringMap{}
, trackItemUpdates{inTrackItemUpdates}
, itemUpdateHistory{}
{
    // Add the null item.
    createItem("Null");
}

const Item* ItemDataBase::createItem(const std::string& displayName)
{
    // Derive the string ID and check that it's unique.
    std::string stringID{deriveStringID(displayName)};
    auto it{itemStringMap.find(stringID)};
    if (it != itemStringMap.end()) {
        // String ID already exists. Do nothing.
        return nullptr;
    }

    // Add the item to the end of each of our vectors.
    ItemID numericID{static_cast<Uint16>(items.size())};
    Item& newItem{items.emplace_back(displayName, stringID, numericID)};
    itemVersions.emplace_back(0);

    return &newItem;
}

const Item* ItemDataBase::updateItem(const Item& newItem)
{
    if (newItem.numericID >= items.size()) {
        LOG_ERROR("Tried to update invalid item. ID: %u", newItem.numericID);
        return nullptr;
    }

    // Derive the string ID and check that it's unique.
    std::string stringID{deriveStringID(newItem.displayName)};
    auto it{itemStringMap.find(stringID)};
    if (it != itemStringMap.end()) {
        // String ID already exists. Do nothing.
        return nullptr;
    }

    // Update the item.
    Item& item{items[newItem.numericID]};
    item = newItem;
    item.stringID = stringID;

    // Increment the version.
    itemVersions[newItem.numericID]++;

    // If we're tracking item updates, add this one to the history.
    if (trackItemUpdates) {
        itemUpdateHistory.emplace_back(newItem.numericID);
    }
}

const Item* ItemDataBase::getItem(const std::string& stringID) const
{
    // Attempt to find the given string ID.
    auto it{itemStringMap.find(stringID)};
    if (it == itemStringMap.end()) {
        return nullptr;
    }

    return it->second;
}

const Item* ItemDataBase::getItem(ItemID numericID) const
{
    if (numericID >= items.size()) {
        LOG_ERROR("Tried to get invalid item. ID: %u", numericID);
        return nullptr;
    }

    return &(items[numericID]);
}

bool ItemDataBase::itemExists(ItemID numericID) const
{
    return (numericID < items.size());
}

ItemVersion ItemDataBase::getItemVersion(ItemID numericID)
{
    if (numericID >= itemVersions.size()) {
        LOG_ERROR("Tried to get invalid item. ID: %u", numericID);
        return 0;
    }

    return itemVersions[numericID];
}

const std::vector<Item>& ItemDataBase::getAllItems() const
{
    return items;
}

const std::vector<ItemID>& ItemDataBase::getItemUpdateHistory()
{
    return itemUpdateHistory;
}

void ItemDataBase::clearItemUpdateHistory()
{
    itemUpdateHistory.clear();
}

std::string ItemDataBase::deriveStringID(const std::string& displayName)
{
    // Make the string all lowercase.
    std::string stringID{displayName};
    std::transform(stringID.begin(), stringID.end(), stringID.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Replace spaces with underscores.
    std::replace(stringID.begin(), stringID.end(), ' ', '_');

    return stringID;
}

} // End namespace AM
