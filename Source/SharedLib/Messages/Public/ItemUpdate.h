#pragma once

#include "EngineMessageType.h"
#include "Item.h"
#include "SharedConfig.h"

namespace AM
{

/**
 * Used by the server to send up-to-date item definitions to the client.
 *
 * Contains only the fields from the Item class that are relevant to clients.
 */
struct ItemUpdate {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemUpdate};

    // Note: See Item.h for more info on all of these.

    /** Unique display name, shown in the UI.  */
    std::string displayName{"Null"};

    /** This item's unique numeric identifier. */
    ItemID numericID{NULL_ITEM_ID};

    /** The ID of this item's icon. */
    IconID iconID{NULL_ICON_ID};

    /** How large a stack of this item can be, e.g. in an inventory slot. */
    Uint8 maxStackSize{1};

    /** The interactions that this item supports. */
    std::vector<ItemInteractionType> supportedInteractions{};

    /** The item's current version number. */
    ItemVersion version{};
};

template<typename S>
void serialize(S& serializer, ItemUpdate& itemUpdate)
{
    serializer.text1b(itemUpdate.displayName, Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.value2b(itemUpdate.numericID);
    serializer.value2b(itemUpdate.iconID);
    serializer.value1b(itemUpdate.maxStackSize);
    serializer.container1b(itemUpdate.supportedInteractions,
                           SharedConfig::MAX_ITEM_CUSTOM_INTERACTIONS);
    serializer.value2b(itemUpdate.version);
}

} // End namespace AM
