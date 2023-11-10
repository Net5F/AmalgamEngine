#pragma once

#include "EngineMessageType.h"
#include "ItemID.h"
#include "NetworkDefs.h"

namespace AM
{

/**
 * Used to request an item's latest definition from the server.
 */
struct ItemRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The item definition that this client is requesting. */
    ItemID itemID{NULL_ITEM_ID};

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
void serialize(S& serializer, ItemRequest& itemRequest)
{
    serializer.value2b(itemRequest.itemID);
}

} // End namespace AM
