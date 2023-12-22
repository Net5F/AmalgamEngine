#pragma once

#include "EngineMessageType.h"
#include "NetworkDefs.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by a client to request that an item be moved to a different slot in 
 * their inventory, or by the server to tell a client that an item was moved.
 *
 * Moving an item into an occupied slot swaps its position with the other item.
 */
struct InventoryMoveItem {
    /** The inventory slot of the item to move. */
    Uint8 sourceSlotIndex{0};

    /** The inventory slot to move the item into (destination). */
    Uint8 destSlotIndex{0};
};

template<typename S>
void serialize(S& serializer, InventoryMoveItem& inventoryMoveItem) {
    serializer.value1b(inventoryMoveItem.sourceSlotIndex);
    serializer.value1b(inventoryMoveItem.destSlotIndex);
}

} // namespace AM
