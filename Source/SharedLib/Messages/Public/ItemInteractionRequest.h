#pragma once

#include "EngineMessageType.h"
#include "NetworkDefs.h"
#include "ItemInteractionType.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Used to request that an interaction be performed.
 */
struct ItemInteractionRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemInteractionRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    // Note: We don't include a tick number in our interaction requests, because
    //       there isn't anything that a client would want to sync the
    //       interaction with. NPC state is in the past, and the client entity's
    //       predicted state (e.g. position) wouldn't be useful to sync to.

    /** The inventory slot of the item that the interaction is being performed
        on. */
    Uint8 slotIndex{0};

    /** The type of interaction to perform.
        Note: UseOn and Destroy are handled by other messages (see
              ItemInteractionType). If you send them using this message, they'll
              be ignored. */
    ItemInteractionType interactionType{};

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
void serialize(S& serializer, ItemInteractionRequest& interactionRequest)
{
    serializer.value1b(interactionRequest.slotIndex);
    serializer.value1b(interactionRequest.interactionType);
}

} // End namespace AM
