#include "Inventory.h"
#include "ItemDataBase.h"

namespace AM
{

Inventory::Inventory(Uint8 inSize)
: size{inSize}
, slots(size)
{
}

bool Inventory::addItem(ItemID itemID, Uint8 count)
{
    // If there's an existing empty slot, fill it with the given item.
    for (ItemSlot& slot : slots) {
        if (slot.ID == NULL_ITEM_ID) {
            slot.ID = itemID;
            slot.count = count;
            return true;
        }
    }

    return false;
}

bool Inventory::removeItem(Uint8 slotIndex, Uint8 count)
{
    // If the slot is invalid or empty, return false.
    if (!slotIndexIsValid(slotIndex) || (slots[slotIndex].count == 0)) {
        return false;
    }

    // Remove the given count of items from the slot.
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
    ItemSlot& sourceSlot{slots[sourceSlotIndex]};
    ItemSlot& destSlot{slots[destSlotIndex]};

    ItemSlot temp{destSlot};
    destSlot = sourceSlot;
    sourceSlot = temp;

    return true;
}

bool Inventory::contains(ItemID itemID) const
{
    for (const ItemSlot& slot : slots) {
        if ((slot.ID == itemID) && (slot.count > 0)) {
            return true;
        }
    }

    return false;
}

ItemID Inventory::getItemID(Uint8 slotIndex) const
{
    // If the slot is invalid or empty, return null.
    if (!slotIndexIsValid(slotIndex) || (slots[slotIndex].count == 0)) {
        return NULL_ITEM_ID;
    }

    return slots[slotIndex].ID;
}

std::size_t Inventory::getItemCount(ItemID itemID) const
{
    std::size_t count{0};
    for (const ItemSlot& slot : slots) {
        if (slot.ID == itemID) {
            count += slot.count;
        }
    }

    return count;
}

const Item* Inventory::getItem(Uint8 slotIndex,
                               const ItemDataBase& itemData) const
{
    // If the slot is invalid or empty, return nullptr.
    if (ItemID itemID{getItemID(slotIndex)}) {
        return itemData.getItem(itemID);
    }
    else {
        return nullptr;
    }
}

const ItemCombination* Inventory::combineItems(Uint8 sourceSlotIndex,
                                               Uint8 targetSlotIndex,
                                               const ItemDataBase& itemData)
{
    // If either slot is invalid or empty, return false.
    if (!slotIndexIsValid(sourceSlotIndex) || !slotIndexIsValid(targetSlotIndex)
        || (slots[sourceSlotIndex].count == 0)
        || (slots[targetSlotIndex].count == 0)) {
        return nullptr;
    }

    // Get the items in the given slots.
    // Note: We don't need to check if they exist since we do so before adding
    //       them to the inventory.
    ItemID sourceItemID{slots[sourceSlotIndex].ID};
    ItemID targetItemID{slots[targetSlotIndex].ID};
    const Item* sourceItem{itemData.getItem(sourceItemID)};
    const Item* targetItem{itemData.getItem(targetItemID)};

    // Try to find a matching combination in either item's list.
    const ItemCombination* matchingCombination{nullptr};
    for (const ItemCombination& combination : sourceItem->itemCombinations) {
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
    if (!slotIndexIsValid(sourceSlotIndex) || !slotIndexIsValid(targetSlotIndex)
        || (slots[sourceSlotIndex].count == 0)
        || (slots[targetSlotIndex].count == 0)) {
        return;
    }

    // Combine the items:
    // Decrement each item's count, erasing them if appropriate.
    reduceItemCount(sourceSlotIndex, 1);
    reduceItemCount(targetSlotIndex, 1);

    // Add the new item.
    addItem(resultItemID, 1);
}

void Inventory::resize(Uint8 newSize)
{
    size = newSize;
    slots.resize(newSize);
}

Uint8 Inventory::getFilledSlotCount()
{
    Uint8 count{0};
    for (ItemSlot& slot : slots) {
        if (slot.ID != NULL_ITEM_ID) {
            count++;
        }
    }

    return count;
}

bool Inventory::slotIndexIsValid(Uint8 slotIndex) const
{
    return (slotIndex < size) && (slotIndex < slots.size());
}

void Inventory::reduceItemCount(Uint8 slotIndex, Uint8 count)
{
    // Reduce the count.
    ItemSlot& itemSlot{slots[slotIndex]};
    itemSlot.count -= count;

    // If the slot is out of items, set it to empty.
    if (itemSlot.count == 0) {
        itemSlot.ID = NULL_ITEM_ID;
        itemSlot.count = 0;
    }
}

} // End namespace AM
