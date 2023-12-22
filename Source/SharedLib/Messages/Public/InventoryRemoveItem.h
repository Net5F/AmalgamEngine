#pragma once

#include "EngineMessageType.h"
#include "ItemID.h"
#include "NetworkDefs.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by a client to request that an item be removed from an inventory, or by 
 * the server to tell a client that an item was removed.
 *
 * Note: This is currently only used for player inventories. If we reuse it for 
 *       NPC inventories, we can add an entity ID field.
 */
struct InventoryRemoveItem {
    /** The inventory slot to remove the item from. */
    Uint8 slotIndex{0};

    /** How many of the item to delete. */
    Uint8 count{0};
};

template<typename S>
void serialize(S& serializer, InventoryRemoveItem& inventoryRemoveItem) {
    serializer.value1b(inventoryRemoveItem.slotIndex);
    serializer.value1b(inventoryRemoveItem.count);
}

} // namespace AM
