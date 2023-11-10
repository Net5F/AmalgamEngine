#pragma once

#include "EngineMessageType.h"
#include "Inventory.h"
#include "ItemID.h"
#include <SDL_stdinc.h>
#include <vector>

namespace AM
{
/**
 * Sent by the server when a player connects and needs their full inventory.
 *
 * Note: This is currently only used for player inventories. If we reuse it for 
 *       NPC inventories, we can add an entity ID field.
 */
struct InventoryInit {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::InventoryInit};

    struct ItemSlot {
        /** The item in this inventory slot. */
        ItemID ID{NULL_ITEM_ID};

        /** How many of the item is in this inventory slot. */
        Uint16 count{0};

        /** The item's version number. Used by the client to tell if it 
            already has the latest definition for this item, or if it needs 
            to request it. */
        ItemVersion version{0};
    };

    std::vector<ItemSlot> items{};
};

template<typename S>
void serialize(S& serializer, InventoryInit::ItemSlot& itemSlot) {
    serializer.value2b(itemSlot.ID);
    serializer.value2b(itemSlot.count);
    serializer.value2b(itemSlot.version);
}

template<typename S>
void serialize(S& serializer, InventoryInit& inventoryInit) {
    serializer.container(inventoryInit.items, Inventory::MAX_ITEMS);
}

} // namespace AM
