#pragma once

#include "EngineMessageType.h"
#include "NetworkDefs.h"
#include "ItemID.h"

namespace AM
{

/**
 * Used to request an item's init script from the server.
 *
 * Init scripts are only requested by clients for use in build mode. Only the
 * server actually runs the scripts.
 */
struct ItemInitScriptRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemInitScriptRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The ID of the item to get the init script for. */
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
void serialize(S& serializer, ItemInitScriptRequest& initScriptRequest)
{
    serializer.value2b(initScriptRequest.itemID);
}

} // End namespace AM
