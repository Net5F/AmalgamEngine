#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Sent by a client to request that two items in a player's inventory be
 * combined.
 */
struct CombineItemsRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::CombineItemsRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    // Note: We don't include a tick number in our interaction requests, because
    //       there isn't anything that a client would want to sync the
    //       interaction with. NPC state is in the past, and the client entity's
    //       predicted state (e.g. position) wouldn't be useful to sync to.

    /** The inventory slot of the item that is being used. */
    Uint8 sourceSlotIndex{0};

    /** The inventory slot of the target item. */
    Uint8 targetSlotIndex{0};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, CombineItemsRequest& combineItemsRequest)
{
    serializer.value1b(combineItemsRequest.sourceSlotIndex);
    serializer.value1b(combineItemsRequest.targetSlotIndex);
}

} // End namespace AM
