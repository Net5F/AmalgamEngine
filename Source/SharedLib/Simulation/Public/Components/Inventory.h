#pragma once

#include "ItemID.h"
#include <SDL_stdinc.h>
#include <vector>

namespace AM
{
class ItemDataBase;
struct Item;
struct ItemCombination;

/**
 * Represents an entity's inventory of items.
 *
 * All client entities have an inventory. Non-client entities may or may not
 * have one.
 */
struct Inventory {
public:
    /** The absolute maximum number of items we can have in an inventory.
        We use Uint8 to hold slot indices, so 256 is our max. */
    static constexpr std::size_t MAX_ITEMS{256};

    /** The default maximum number of items we can have in an inventory. */
    static constexpr Uint8 DEFAULT_INVENTORY_SIZE{20};

    /** The number of slots in this inventory. */
    Uint8 size;

    struct ItemSlot {
        /** The item in this inventory slot. */
        ItemID ID{NULL_ITEM_ID};

        /** How many of the item is in this inventory slot. */
        Uint8 count{0};
    };

    /** This inventory's item slots.
        Empty slots will have ID == NULL_ITEM_ID.
        Note: Slots may be allocated, but still be empty. If you're iterating
              this vector, be sure to check for NULL_ITEM_ID. */
    std::vector<ItemSlot> slots;

    Inventory(Uint8 inSize = DEFAULT_INVENTORY_SIZE);

    // Note: You likely don't want to call these directly.
    //       See InventoryHelpers.h
    /**
     * Adds the given item to the first available slot.
     *
     * If no empty slot is available, adds one to the end.
     *
     * @return true if the item was added, else false (inventory is full).
     *
     * Note: This doesn't check if itemID is a valid item.
     */
    bool addItem(ItemID itemID, Uint8 count);

    /**
     * Removes the given count of items from the given inventory slot.
     *
     * @return true if the item was removed, else false (slot index isn't valid,
     *         count is 0).
     */
    bool removeItem(Uint8 slotIndex, Uint8 count);

    /**
     * Moves the item in sourceSlot into destSlot (or swaps, if there's an
     * item already in destSlot).
     *
     * @return true if the items were moved, else false (slot index isn't
     * valid).
     */
    bool moveItem(Uint8 sourceSlotIndex, Uint8 destSlotIndex);

    /**
     * Returns true if this inventory contains at least 1 of the given item.
     */
    bool contains(ItemID itemID) const;

    /**
     * Returns the ID of the item at the given inventory slot.
     * If the given index is invalid or there's no item in the slot, returns
     * NULL_ITEM_ID.
     */
    ItemID getItemID(Uint8 slotIndex) const;

    /**
     * Returns the count for the given item across all inventory slots.
     */
    std::size_t getItemCount(ItemID itemID) const;

    /**
     * Returns the item at the given inventory slot.
     * If the given index is invalid or there's no item in the slot, returns
     * nullptr.
     */
    const Item* getItem(Uint8 slotIndex, const ItemDataBase& itemData) const;

    /**
     * Combines the items in the given slots and decrements their count
     * (emptying the slot if count == 0).
     *
     * Looks up the item's combinations to determine what the resulting item is.
     * Only for use by the server.
     *
     * @return The used combination if the items were combined, else nullptr
     *         (slot index isn't valid, either slot was empty, neither item
     *         contained the combination).
     */
    const ItemCombination* combineItems(Uint8 sourceSlotIndex,
                                        Uint8 targetSlotIndex,
                                        const ItemDataBase& itemData);

    /**
     * Overload for the client version of this operation.
     *
     * Clients don't have the combination data for any items, so this overload
     * instead takes in the resulting item's ID.
     */
    void combineItems(Uint8 sourceSlotIndex, Uint8 targetSlotIndex,
                      ItemID resultItemID);

    /**
     * Resizes this inventory to match the given new size.
     */
    void resize(Uint8 newSize);

    /**
     * Returns the number of slots that have an item in them.
     */
    Uint8 getFilledSlotCount();

    /** Returns true if the given inventory slot index is valid, else returns
        false. */
    bool slotIndexIsValid(Uint8 slotIndex) const;

    /**
     * Reduces the item count in the given slot by the given count.
     * If the resulting count <= 0, empties the slot.
     */
    void reduceItemCount(Uint8 slotIndex, Uint8 count);
};

} // namespace AM
