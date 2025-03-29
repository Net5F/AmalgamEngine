#include "ItemDataBase.h"
#include "ItemID.h"
#include "StringTools.h"
#include "Log.h"
#include "AMAssert.h"

namespace
{
/** A scratch buffer used while processing string IDs.
    Must be file-local so it can be accessed by const functions. */
std::string workStringID{};
}

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
    // Note: We intentionally don't have a "null item", because we want the 
    //       getters to return nullptr if there isn't a real item to return.
}

const Item* ItemDataBase::loadItem(const Item& referenceItem,
                                   ItemVersion version)
{
    // Check if the item already exists, so we can signal properly later.
    bool itemExisted{itemMap.find(referenceItem.numericID) != itemMap.end()};

    // Add or update the item in our maps.
    // Note: When we insert into an unordered_map, references to the map's
    //       elements are guaranteed to remain valid (for itemStringMap).
    itemMap[referenceItem.numericID] = referenceItem;
    Item& item{itemMap[referenceItem.numericID]};
    itemVersionMap[item.numericID] = version;
    itemStringMap[item.stringID] = &item;

    // Always update nextItemID to be 1 greater than the highest ID.
    if (item.numericID >= nextItemID) {
        nextItemID = (item.numericID + 1);
    }

    // Signal that an item has been updated or created.
    if (itemExisted) {
        itemUpdatedSig.publish(item.numericID);
    }
    else {
        itemCreatedSig.publish(item.numericID);
    }

    return &item;
}

const Item* ItemDataBase::getItem(std::string_view stringID) const
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

ItemVersion ItemDataBase::getItemVersion(ItemID numericID) const
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
