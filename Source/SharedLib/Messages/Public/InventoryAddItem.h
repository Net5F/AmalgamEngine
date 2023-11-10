#pragma once

#include "EngineMessageType.h"
#include "ItemID.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by a client to request that an item be added to an inventory, or by the 
 * server to tell a client that an item was added.
 */
struct InventoryAddItem {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::InventoryAddItem};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The entity to add the item to. */
    entt::entity entity{entt::null};

    /** The item to add. */
    ItemID itemID{NULL_ITEM_ID};

    /** How many of the item to add. */
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
void serialize(S& serializer, InventoryAddItem& inventoryAddItem) {
    serializer.value4b(inventoryAddItem.entityToAddTo);
    serializer.value2b(inventoryAddItem.itemID);
    serializer.value2b(inventoryAddItem.count);
}

} // namespace AM
