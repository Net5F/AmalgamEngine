#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
#include "ItemInitScript.h"
#include "ItemID.h"
#include <string>

namespace AM
{

/**
 * Used to send an entity's init script to a client.
 *
 * Init scripts are only requested by clients for use in build mode. Only the
 * server actually runs the scripts.
 *
 * Note: This is named "Response" to differentiate it from the ItemInitScript
 *       class. Normally we don't append "Response" to message names.
 */
struct ItemInitScriptResponse {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemInitScriptResponse};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The ID of the item that this init script is for. */
    ItemID itemID{NULL_ITEM_ID};

    /** This item's init script. */
    ItemInitScript initScript{};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, ItemInitScriptResponse& initScriptResponse)
{
    serializer.value2b(initScriptResponse.itemID);
    serializer.object(initScriptResponse.initScript);
}

} // End namespace AM
