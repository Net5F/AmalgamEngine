#pragma once

#include "EngineMessageType.h"
#include "Item.h"

namespace AM
{

/**
 * Used by the server to send item definitions to the client.
 */
struct ItemUpdate {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemUpdate};

    // Note: See Item.h for more info on all of these.

    /** Unique display name, shown in the UI.  */
    std::string displayName{"Null"};

    /** The item's unique string ID. */
    std::string stringID{"null"};

    /** This item's unique numeric identifier. */
    ItemID numericID{NULL_ITEM_ID};

    /** The ID of this item's icon. */
    IconID iconID{NULL_ICON_ID};

    /** The interactions that this item supports. */
    std::array<ItemInteractionType, Item::MAX_CUSTOM_INTERACTIONS>
        supportedInteractions{};
};

template<typename S>
void serialize(S& serializer, ItemUpdate& itemUpdate)
{
    serializer.text1b(itemUpdate.displayName, Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.text1b(itemUpdate.stringID, Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.value2b(itemUpdate.numericID);
    serializer.value2b(itemUpdate.iconID);
    serializer.container1b(itemUpdate.supportedInteractions);
}

} // End namespace AM
