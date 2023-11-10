#pragma once

#include "EngineMessageType.h"
#include "ItemID.h"
#include "NetworkDefs.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by a client to request that an item be deleted from an inventory, or by 
 * the server to tell a client that an item was deleted.
 *
 * Note: This is currently only used for player inventories. If we reuse it for 
 *       NPC inventories, we can add an entity ID field.
 */
struct InventoryDeleteItem {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::InventoryDeleteItem};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The inventory slot to delete the item from. */
    Uint8 slotIndex{0};

    /** How many of the item to delete. */
    Uint16 count{0};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /**
     * The network ID of the client that sent this message.
     * Set by the server.
     * No IDs are accepted from the client because we can't trust it,
     * so we fill in the ID based on which socket the message came from.
     */
    NetworkID netID{0};
};

template<typename S>
void serialize(S& serializer, InventoryDeleteItem& inventoryDeleteItem) {
    serializer.value1b(inventoryDeleteItem.slotIndex);
    serializer.value2b(inventoryDeleteItem.count);
}

} // namespace AM
