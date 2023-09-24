#pragma once

#include "EngineMessageType.h"
#include "AnimationState.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Used by clients to request animation state changes on the server.
 */
struct AnimationStateChangeRequest {
    // The enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::AnimationStateChangeRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The entity to change the animation state of. */
    entt::entity entity{entt::null};

    /** The new animation state. */
    AnimationState animationState;

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
void serialize(S& serializer,
               AnimationStateChangeRequest& animationStateChangeRequest)
{
    serializer.value4b(animationStateChangeRequest.entity);
    serializer.object(animationStateChangeRequest.animationState);
}

} // End namespace AM
