#include "Inventory.h"
#include "ItemDataBase.h"

namespace AM
{

bool Inventory::addItem(ItemID itemID, Uint16 count)
{
    // Try to find an empty slot.
    bool emptySlotFound{false};
    for (ItemSlot& item : items) {
        if (item.ID == NULL_ITEM_ID) {
            item.ID = itemID;
            item.count = count;
            emptySlotFound = true;
        }
    }

    // If an empty slot was found, add the item.
    if (!emptySlotFound && (items.size() < MAX_ITEMS)) {
        items.emplace_back(itemID, count);
        return true;
    }

    return false;
}

bool Inventory::deleteItem(Uint8 slotIndex, Uint16 count)
{
    // If the slot index is invalid or empty, return false.
    if (!slotIndexIsValid(slotIndex)
        || (items[slotIndex].count == 0)) {
        return false;
    }

    // Delete the given count of items from the slot.
    reduceItemCount(slotIndex, count);

    return true;
}

bool Inventory::moveItem(Uint8 sourceSlotIndex, Uint8 destSlotIndex)
{
    // If either slot is invalid, return false.
    if (!slotIndexIsValid(sourceSlotIndex)
        || !slotIndexIsValid(destSlotIndex)) {
        return false;
    }

    // Swap the slots.
    ItemSlot& sourceSlot{items[sourceSlotIndex]};
    ItemSlot& destSlot{items[destSlotIndex]};

    ItemSlot temp{destSlot};
    destSlot = sourceSlot;
    sourceSlot = temp;

    return true;
}

bool Inventory::combineItems(Uint8 sourceSlotIndex, Uint8 targetSlotIndex,
                             const ItemDataBase& itemData)
{
    // If either slot is invalid or empty, return false.
    if (!slotIndexIsValid(sourceSlotIndex)
        || !slotIndexIsValid(targetSlotIndex)
        || (items[sourceSlotIndex].count == 0)
        || (items[targetSlotIndex].count == 0)) {
        return false;
    }

    // Get the items in the given slots.
    // Note: We don't need to check if they exist since we do so before adding 
    //       them to the inventory.
    ItemID sourceItemID{items[sourceSlotIndex].ID};
    ItemID targetItemID{items[targetSlotIndex].ID};
    const Item* sourceItem{itemData.getItem(sourceItemID)};
    const Item* targetItem{itemData.getItem(targetItemID)};

    // If either item has this combination listed, find the resulting item.
    ItemID resultItemID{NULL_ITEM_ID};
    for (const Item::ItemCombination& combination :
         sourceItem->itemCombinations) {
        if (combination.otherItemID == targetItemID) {
            resultItemID = combination.resultItemID;
            break;
        }
    }
    if (resultItemID == NULL_ITEM_ID) {
        for (const Item::ItemCombination& combination :
             targetItem->itemCombinations) {
            if (combination.otherItemID == sourceItemID) {
                resultItemID = combination.resultItemID;
                break;
            }
        }
    }

    // If we found a resulting item, combine the items.
    if (resultItemID != NULL_ITEM_ID) {
        // Decrement each item's count, erasing them if appropriate.
        reduceItemCount(sourceSlotIndex, 1);
        reduceItemCount(targetSlotIndex, 1);

        // Add the new item.
        addItem(resultItemID, 1);
    }

    return true;
}

bool Inventory::slotIndexIsValid(Uint8 slotIndex)
{ 
    return (slotIndex < MAX_ITEMS) && (slotIndex < items.size());
}

void Inventory::reduceItemCount(Uint8 slotIndex, Uint16 count)
{
    // Reduce the count.
    ItemSlot& itemSlot{items[slotIndex]};
    itemSlot.count -= count;

    // If the slot is out of items, set it to empty.
    if (itemSlot.count <= 0) {
        itemSlot.ID = NULL_ITEM_ID;
        itemSlot.count = 0;
    }
}

} // End namespace AM
