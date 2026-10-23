#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
#include "ItemID.h"
#include "IconID.h"
#include "ItemInitScript.h"
#include "Item.h"
#include "NetworkDefs.h"
#include <string>

namespace AM
{
/**
 * Sent by the client to request that a new item be created.
 */
struct ItemInitRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemInitRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The item's display name. Not guaranteed to be unique. Shown in the UI.
     */
    std::string displayName{"Null"};

    /** The item's unique string ID.  */
    std::string stringID{""};

    /** The ID of this item's icon. */
    IconID iconID{NULL_ICON_ID};

    /** The script to run on this item after creation. */
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
void serialize(S& serializer, ItemInitRequest& itemInitRequest)
{
    serializer.text1b(itemInitRequest.displayName,
                      Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.text1b(itemInitRequest.stringID, Item::MAX_STRING_ID_LENGTH);
    serializer.value2b(itemInitRequest.iconID);
    serializer.object(itemInitRequest.initScript);
}

} // End namespace AM
