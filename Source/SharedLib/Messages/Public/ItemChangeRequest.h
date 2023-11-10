#pragma once

#include "EngineMessageType.h"
#include "Item.h"
#include "NetworkDefs.h"

namespace AM
{

/**
 * Used to request an item definition be changed. 
 */
struct ItemChangeRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemChangeRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The new item definition.
        The numericID of this item determines which item should be updated. 
        Note: The updated item's stringID will be derived from this item's 
              displayName, regardless of what this item's stringID is. */
    Item item{};

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
void serialize(S& serializer, ItemChangeRequest& itemChangeRequest)
{
    serializer.object(itemChangeRequest.item);
}

} // End namespace AM
