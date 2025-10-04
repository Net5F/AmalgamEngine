#pragma once

#include "EngineMessageType.h"
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
    /** The item's display name.
        The item's stringID will be derived from this. */
    std::string displayName{""};

    /** The ID of this item's icon. */
    IconID iconID{NULL_ICON_ID};

    /** The script to run on this item after creation. */
    ItemInitScript initScript{};

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
void serialize(S& serializer, ItemInitRequest& itemInitRequest)
{
    serializer.text1b(itemInitRequest.displayName,
                      Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.value2b(itemInitRequest.iconID);
    serializer.object(itemInitRequest.initScript);
}

} // End namespace AM
