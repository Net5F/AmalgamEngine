#pragma once

#include "ItemID.h"
#include <SDL_stdinc.h>
#include <vector>

namespace AM
{
class ItemDataBase;

/**
 * Represents an entity's inventory of items.
 */
struct Inventory {
    /** The maximum number of items we can have in an inventory. 
        We use Uint8 to hold slot indices, so 256 is our max. */
    static constexpr std::size_t MAX_ITEMS{256};

    struct ItemSlot {
        /** The item in this inventory slot. */
        ItemID ID{NULL_ITEM_ID};

        /** How many of the item is in this inventory slot. */
        Uint16 count{0};
    };

    /** Holds this inventory's items.
        Empty slots will have ID == NULL_ITEM_ID. */
    std::vector<ItemSlot> items{};

    /**
     * Adds the given item to the first available slot.
     *
     * If no empty slot is available, adds one to the end.
     *
     * @return true if the item was added, else false (inventory is full).
     *
     * Note: This doesn't check if itemID is a valid item.
     */
    bool addItem(ItemID itemID, Uint16 count);

    /**
     * Deletes the given count of items from the given inventory slot.
     *
     * @return true if the item was deleted, else false (slot index isn't valid).
     */
    bool deleteItem(Uint8 slotIndex, Uint16 count);

    /**
     * Moves the item in sourceSlot into destSlot (or swaps, if there's an 
     * item already in destSlot).
     *
     * @return true if the items were moved, else false (slot index isn't valid).
     */
    bool moveItem(Uint8 sourceSlotIndex, Uint8 destSlotIndex);

    /**
     * Combines the items in the given slots and decrements their count 
     * (emptying the slot if count == 0).
     * 
     * @return true if the items were combined, else false (slot index isn't 
     *         valid, either slot was empty, neither item contained the 
     *         combination).
     */
    bool combineItems(Uint8 sourceSlotIndex, Uint8 targetSlotIndex,
                      const ItemDataBase& itemData);

    /** Returns true if the given inventory slot index is valid, else returns 
        false. */
    bool slotIndexIsValid(Uint8 slotIndex);

private:
    /**
     * Reduces the item count in the given slot by the given count.
     * If the resulting count <= 0, empties the slot.
     */
    void reduceItemCount(Uint8 slotIndex, Uint16 count);
};

} // namespace AM
