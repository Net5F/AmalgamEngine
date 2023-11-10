#pragma once

#include "EngineMessageType.h"
#include "NetworkDefs.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * Sent by a client to request that two items in a player's inventory be 
 * combined, or by the server to tell a client that items in their inventory 
 * were combined.
 */
struct CombineItems {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::CombineItems};
  
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
    /**
     * The network ID of the client that sent this message.
     * Set by the server.
     * No IDs are accepted from the client because we can't trust it, so we 
     * fill in the ID based on which socket the message came from.
     */
    NetworkID netID{0};
};

template<typename S>
void serialize(S& serializer, CombineItems& combineItems)
{
    serializer.value1b(combineItems.sourceSlotIndex);
    serializer.value1b(combineItems.targetSlotIndex);
}

} // End namespace AM
