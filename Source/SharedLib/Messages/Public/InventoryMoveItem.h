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
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::InventoryMoveItem};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The inventory slot of the item to move. */
    Uint8 sourceSlotIndex{0};

    /** The inventory slot to move the item into (destination). */
    Uint8 destSlotIndex{0};

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
void serialize(S& serializer, InventoryMoveItem& inventoryMoveItem) {
    serializer.value1b(inventoryMoveItem.sourceSlotIndex);
    serializer.value1b(inventoryMoveItem.destSlotIndex);
}

} // namespace AM
