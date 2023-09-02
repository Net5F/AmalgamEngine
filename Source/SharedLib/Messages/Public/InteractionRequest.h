#pragma once

#include "EngineMessageType.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{

// Note: When we add item interactions, we may want to rename this to 
//       EntityInteractionRequest and do a separate ItemInteractionRequest.

/**
 * Used to request that an interaction be performed.
 */
struct InteractionRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::InteractionRequest};
  
    // Note: We don't include a tick number in our interaction requests, because 
    //       there isn't anything that a client would want to sync the 
    //       interaction with. NPC state is in the past, and the client entity's 
    //       predicted state (e.g. position) wouldn't be useful to sync to.

    /** The ID of the client entity performing the interaction. */
    entt::entity clientEntity{entt::null};

    /** The ID of the entity that the interaction is being performed on. */
    entt::entity targetEntity{entt::null};

    /** The type of interaction to perform.
        Note: This should either be cast to EngineInteractionType or 
              ProjectInteractionType, depending on its value. */
    Uint8 interactionType{};

    //--------------------------------------------------------------------------
    // Non-replicated data
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
void serialize(S& serializer, InteractionRequest& interactionRequest)
{
    serializer.value4b(interactionRequest.clientEntity);
    serializer.value4b(interactionRequest.targetEntity);
    serializer.value1b(interactionRequest.interactionType);
}

} // End namespace AM
