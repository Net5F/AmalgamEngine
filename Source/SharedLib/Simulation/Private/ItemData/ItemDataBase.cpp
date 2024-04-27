#include "ItemDataBase.h"
#include "ItemID.h"
#include "StringTools.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
ItemDataBase::ItemDataBase()
: itemMap{}
, itemStringMap{}
, itemVersionMap{}
, nextItemID{NULL_ITEM_ID + 1}
, workStringID{}
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
    StringTools::deriveStringID(item.displayName, workStringID);
    if (itemStringMap.find(workStringID) != itemStringMap.end()) {
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
    newItem.stringID = workStringID;
    newItem.numericID = newItemID;
    itemVersionMap[newItemID] = 0;
    itemStringMap[workStringID] = &newItem;

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
    StringTools::deriveStringID(newItem.displayName, workStringID);
    Item& item{itemIt->second};
    if (workStringID != item.stringID) {
        // If the new ID is taken, do nothing.
        auto stringIt{itemStringMap.find(workStringID)};
        if (stringIt != itemStringMap.end()) {
            return nullptr;
        }
        else {
            // New ID isn't taken. Add it to the string ID map and remove the
            // old one.
            itemStringMap[workStringID] = &item;
            itemStringMap.erase(item.stringID);
        }
    }

    // Update the item.
    item = newItem;
    item.stringID = workStringID;

    // Increment the version.
    itemVersionMap[newItem.numericID]++;

    // Signal that an item has been updated.
    itemUpdatedSig.publish(newItem.numericID);

    return &item;
}

const Item* ItemDataBase::getItem(std::string_view stringID)
{
    // Derive string ID in case the user accidentally passed a display name.
    StringTools::deriveStringID(stringID, workStringID);

    // Attempt to find the string ID.
    auto it{itemStringMap.find(workStringID)};
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

} // End namespace AM
