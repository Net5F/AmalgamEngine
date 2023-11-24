#include "ItemDataBase.h"
#include "ItemID.h"
#include "Log.h"
#include "AMAssert.h"
#include <algorithm>

namespace AM
{
ItemDataBase::ItemDataBase(bool inTrackItemUpdates)
: itemMap{}
, itemStringMap{}
, itemVersionMap{}
, trackItemUpdates{inTrackItemUpdates}
, itemUpdateHistory{}
, nextItemID{0}
{
    // Add the null item.
    createItem(Item{"Null", "", NULL_ITEM_ID});
}

const Item* ItemDataBase::createItem(const Item& item)
{
    // If the numeric ID is taken, do nothing.
    if ((item.numericID != NULL_ITEM_ID)
        && itemMap.find(item.numericID) != itemMap.end()) {
        return nullptr;
    }

    // Derive the string ID. If it's taken, do nothing.
    std::string stringID{deriveStringID(item.displayName)};
    if (itemStringMap.find(stringID) != itemStringMap.end()) {
        return nullptr;
    }

    // If the new item doesn't have a desired ID, use the next sequential ID.
    ItemID newItemID{item.numericID};
    if (newItemID == NULL_ITEM_ID) {
        newItemID = nextItemID;
    }

    // Add the item to our maps.
    // Note: When we insert into an unordered_map, references to the map's 
    //       elements are guaranteed to remain valid (for itemStringMap).
    itemMap[newItemID] = item;
    Item& newItem{itemMap[newItemID]};
    newItem.stringID = stringID;
    newItem.numericID = newItemID;
    itemVersionMap[newItemID] = 0;
    itemStringMap[stringID] = &newItem;

    // Always update nextItemID to be 1 greater than the highest ID.
    if (newItemID >= nextItemID) {
        nextItemID = (newItemID + 1);
    }

    return &newItem;
}

const Item* ItemDataBase::updateItem(const Item& newItem)
{
    // If the item doesn't exist, do nothing.
    auto itemIt{itemMap.find(newItem.numericID)};
    if (itemIt == itemMap.end()) {
        return nullptr;
    }

    // If the new derived string ID doesn't match the old one.
    std::string stringID{deriveStringID(newItem.displayName)};
    Item& item{itemIt->second};
    if (stringID != item.stringID) {
        // If the new ID is taken, do nothing.
        auto stringIt{itemStringMap.find(stringID)};
        if (stringIt != itemStringMap.end()) {
            return nullptr;
        }
        else {
            // New ID isn't taken. Add it to the string ID map.
            itemStringMap[stringID] = &item;
        }
    }

    // TODO: Need to update the string ID in the map

    // Update the item.
    item = newItem;
    item.stringID = stringID;

    // Increment the version.
    itemVersionMap[newItem.numericID]++;

    // If we're tracking item updates, add this one to the history.
    if (trackItemUpdates) {
        itemUpdateHistory.emplace_back(newItem.numericID);
    }

    return &item;
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
    // Attempt to find the given numeric ID.
    auto it{itemMap.find(numericID)};
    if (it == itemMap.end()) {
        return nullptr;
    }

    return &(it->second);
}

bool ItemDataBase::itemExists(ItemID numericID) const
{
    return (itemMap.find(numericID) != itemMap.end());
}

ItemVersion ItemDataBase::getItemVersion(ItemID numericID)
{
    // Attempt to find the given numeric ID.
    auto it{itemVersionMap.find(numericID)};
    if (it == itemVersionMap.end()) {
        LOG_ERROR("Tried to get invalid item's version. ID: %u", numericID);
        return 0;
    }

    return it->second;
}

const std::unordered_map<ItemID, Item>& ItemDataBase::getAllItems() const
{
    return itemMap;
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
