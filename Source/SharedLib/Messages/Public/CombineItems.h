#pragma once

#include "EngineMessageType.h"
#include "ItemID.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * Sent by the server to tell a client that items in their inventory were
 * combined.
 */
struct CombineItems {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::CombineItems};

    /** The inventory slot of the item that is being used. */
    Uint8 sourceSlotIndex{0};

    /** The inventory slot of the target item. */
    Uint8 targetSlotIndex{0};

    /** The resulting item. */
    ItemID resultItemID{NULL_ITEM_ID};

    /** The resulting item's version number. Used by the client to tell if it
        already has the latest definition for this item, or if it needs
        to request it. */
    ItemVersion resultItemVersion{0};
};

template<typename S>
void serialize(S& serializer, CombineItems& combineItems)
{
    serializer.value1b(combineItems.sourceSlotIndex);
    serializer.value1b(combineItems.targetSlotIndex);
    serializer.value2b(combineItems.resultItemID);
    serializer.value2b(combineItems.resultItemVersion);
}

} // End namespace AM
