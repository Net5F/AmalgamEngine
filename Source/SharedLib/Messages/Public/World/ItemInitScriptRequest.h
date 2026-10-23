#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
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
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, ItemInitScriptRequest& initScriptRequest)
{
    serializer.value2b(initScriptRequest.itemID);
}

} // End namespace AM
