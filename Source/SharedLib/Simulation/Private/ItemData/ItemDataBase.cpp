#include "ItemDataBase.h"
#include "ItemID.h"
#include "Log.h"
#include "AMAssert.h"
#include <algorithm>

namespace AM
{
ItemDataBase::ItemDataBase()
: itemMap{}
, itemStringMap{}
, itemVersionMap{}
, nextItemID{NULL_ITEM_ID + 1}
, itemCreatedSig{}
, itemUpdatedSig{}
, itemCreated{itemCreatedSig}
, itemUpdated{itemUpdatedSig}
{
}

const Item* ItemDataBase::createItem(const Item& item)
{
    // If the numeric ID is taken, do nothing.
    if (itemMap.find(item.numericID) != itemMap.end()) {
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

    // Signal that an item has been created.
    itemCreatedSig.publish(newItemID);

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
            // New ID isn't taken. Add it to the string ID map and remove the
            // old one.
            itemStringMap[stringID] = &item;
            itemStringMap.erase(item.stringID);
        }
    }

    // Update the item.
    item = newItem;
    item.stringID = stringID;

    // Increment the version.
    itemVersionMap[newItem.numericID]++;

    // Signal that an item has been updated.
    itemUpdatedSig.publish(newItem.numericID);

    return &item;
}

const Item* ItemDataBase::getItem(std::string_view stringID) const
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

std::string ItemDataBase::deriveStringID(std::string_view displayName)
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
