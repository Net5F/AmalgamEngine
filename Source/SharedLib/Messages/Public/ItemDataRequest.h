#pragma once

#include "EngineMessageType.h"
#include "ItemID.h"
#include "Item.h"
#include "NetworkDefs.h"
#include "bitsery/ext/std_variant.h"
#include <variant>
#include <string>

namespace AM
{

/**
 * Used to request an item's current state from the server.
 */
struct ItemDataRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemDataRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The stringID or numericID of the item definition that this client is 
        requesting. */
    std::variant<std::string, ItemID> itemID{};

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
void serialize(S& serializer, ItemDataRequest& itemDataRequest)
{
    serializer.ext(
        itemDataRequest.itemID,
        bitsery::ext::StdVariant{
            [](S& serializer, std::string& stringID) {
               serializer.text1b(stringID,
                                 Item::MAX_DISPLAY_NAME_LENGTH);
            },
            [](S& serializer, ItemID& numericID) {
                serializer.value2b(numericID);
            }
    });
}

} // End namespace AM
