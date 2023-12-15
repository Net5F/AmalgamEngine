#include "Inventory.h"
#include "ItemDataBase.h"

namespace AM
{

bool Inventory::addItem(ItemID itemID, Uint16 count)
{
    // If there's an existing empty slot, fill it with the given item.
    for (ItemSlot& item : items) {
        if (item.ID == NULL_ITEM_ID) {
            item.ID = itemID;
            item.count = count;
            return true;
        }
    }

    // If an empty slot wasn't found, push a new slot into the vector.
    if (items.size() < MAX_ITEMS) {
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

ItemID Inventory::getItemID(Uint8 slotIndex) const
{
    // If the slot index is invalid or empty, return false.
    if (!slotIndexIsValid(slotIndex)
        || (items[slotIndex].count == 0)) {
        return NULL_ITEM_ID;
    }

    return items[slotIndex].ID;
}

const Item* Inventory::getItem(Uint8 slotIndex,
                               const ItemDataBase& itemData) const
{
    // If the slot index is invalid or empty, return nullptr.
    if (ItemID itemID{getItemID(slotIndex)}) {
        return itemData.getItem(itemID);
    }
    else {
        return nullptr;
    }
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

const ItemCombination* Inventory::combineItems(Uint8 sourceSlotIndex,
                                               Uint8 targetSlotIndex,
                                               const ItemDataBase& itemData)
{
    // If either slot is invalid or empty, return false.
    if (!slotIndexIsValid(sourceSlotIndex)
        || !slotIndexIsValid(targetSlotIndex)
        || (items[sourceSlotIndex].count == 0)
        || (items[targetSlotIndex].count == 0)) {
        return nullptr;
    }

    // Get the items in the given slots.
    // Note: We don't need to check if they exist since we do so before adding 
    //       them to the inventory.
    ItemID sourceItemID{items[sourceSlotIndex].ID};
    ItemID targetItemID{items[targetSlotIndex].ID};
    const Item* sourceItem{itemData.getItem(sourceItemID)};
    const Item* targetItem{itemData.getItem(targetItemID)};

    // Try to find a matching combination in either item's list.
    const ItemCombination* matchingCombination{nullptr};
    for (const ItemCombination& combination :
         sourceItem->itemCombinations) {
        if (combination.otherItemID == targetItemID) {
            matchingCombination = &combination;
            break;
        }
    }
    if (!matchingCombination) {
        for (const ItemCombination& combination :
             targetItem->itemCombinations) {
            if (combination.otherItemID == sourceItemID) {
                matchingCombination = &combination;
                break;
            }
        }
    }

    // If we found a resulting item, combine the items.
    if (matchingCombination) {
        // Decrement each item's count, erasing them if appropriate.
        reduceItemCount(sourceSlotIndex, 1);
        reduceItemCount(targetSlotIndex, 1);

        // Add the new item.
        addItem(matchingCombination->resultItemID, 1);
    }

    return matchingCombination;
}

void Inventory::combineItems(Uint8 sourceSlotIndex, Uint8 targetSlotIndex,
    ItemID resultItemID)
{
    // If either slot is invalid or empty, return false.
    if (!slotIndexIsValid(sourceSlotIndex)
        || !slotIndexIsValid(targetSlotIndex)
        || (items[sourceSlotIndex].count == 0)
        || (items[targetSlotIndex].count == 0)) {
        return;
    }

    // Combine the items:
    // Decrement each item's count, erasing them if appropriate.
    reduceItemCount(sourceSlotIndex, 1);
    reduceItemCount(targetSlotIndex, 1);

    // Add the new item.
    addItem(resultItemID, 1);
}

Uint8 Inventory::getFilledSlotCount()
{
    Uint8 count{0};
    for (ItemSlot& item : items) {
        if (item.ID != NULL_ITEM_ID) {
            count++;
        }
    }

    return count;
}

bool Inventory::slotIndexIsValid(Uint8 slotIndex) const
{ 
    return (slotIndex < MAX_ITEMS) && (slotIndex < items.size());
}

void Inventory::reduceItemCount(Uint8 slotIndex, Uint16 count)
{
    // Reduce the count.
    ItemSlot& itemSlot{items[slotIndex]};
    itemSlot.count -= count;

    // If the slot is out of items, set it to empty.
    if (itemSlot.count == 0) {
        itemSlot.ID = NULL_ITEM_ID;
        itemSlot.count = 0;
    }
}

} // End namespace AM
