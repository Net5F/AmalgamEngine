#include "ItemData.h"
#include "StringTools.h"
#include "Log.h"
#include <algorithm>

namespace
{
/** A scratch buffer used while processing string IDs.
    Must be file-local so it can be accessed by const functions. */
std::string workStringID{};
}

namespace AM
{
namespace Server
{
ItemData::ItemData()
: ItemDataBase()
, itemInitScriptMap{}
, defaultInitScript{}
{
}

const Item* ItemData::createItem(const Item& referenceItem,
                                 std::string_view initScript)
{
    // If the numeric ID is taken, do nothing.
    if (itemMap.find(referenceItem.numericID) != itemMap.end()) {
        return nullptr;
    }

    // Derive the string ID. If it's taken, do nothing.
    StringTools::deriveStringID(referenceItem.displayName, workStringID);
    if (itemStringMap.find(workStringID) != itemStringMap.end()) {
        return nullptr;
    }

    // If referenceItem doesn't have a desired ID, use the next sequential ID.
    ItemID newItemID{referenceItem.numericID};
    if (newItemID == NULL_ITEM_ID) {
        newItemID = nextItemID;
    }

    // Add the item to our maps.
    // Note: When we insert into an unordered_map, references to the map's
    //       elements are guaranteed to remain valid (for itemStringMap).
    itemMap[newItemID] = referenceItem;
    Item& newItem{itemMap[newItemID]};
    newItem.stringID = workStringID;
    newItem.numericID = newItemID;
    itemStringMap[workStringID] = &newItem;
    itemVersionMap[newItemID] = 0;
    itemInitScriptMap[newItemID] = {std::string{initScript}};

    // Always update nextItemID to be 1 greater than the highest ID.
    if (newItemID >= nextItemID) {
        nextItemID = (newItemID + 1);
    }

    // Signal that an item has been created.
    itemCreatedSig.publish(newItemID);

    return &newItem;
}

const Item* ItemData::updateItem(const Item& referenceItem,
                                 std::string_view initScript)
{
    // If the item doesn't exist, do nothing.
    auto itemIt{itemMap.find(referenceItem.numericID)};
    if (itemIt == itemMap.end()) {
        return nullptr;
    }

    // If the new derived string ID doesn't match the old one.
    StringTools::deriveStringID(referenceItem.displayName, workStringID);
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
    item = referenceItem;
    item.stringID = workStringID;
    itemInitScriptMap[item.numericID] = {std::string{initScript}};

    // Increment the version.
    itemVersionMap[item.numericID]++;

    // Signal that an item has been updated.
    itemUpdatedSig.publish(item.numericID);

    return &item;
}

const Item* ItemData::loadItem(const Item& referenceItem, ItemVersion version,
                               std::string_view initScript)
{
    // If the item was successfully loaded, add the init script to our map.
    const Item* item{ItemDataBase::loadItem(referenceItem, version)};
    if (item) {
        itemInitScriptMap[item->numericID] = {std::string{initScript}};
    }

    return item;
}

const ItemInitScript& ItemData::getItemInitScript(ItemID numericID) const
{
    // Attempt to find the given numeric ID.
    auto it{itemInitScriptMap.find(numericID)};
    if (it == itemInitScriptMap.end()) {
        LOG_ERROR("Tried to get invalid item's init script. ID: %u", numericID);
        return defaultInitScript;
    }

    return it->second;
}

} // End namespace Server
} // End namespace AM
