#pragma once

#include "EngineMessageType.h"
#include "Name.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Used by clients to request name changes on the server.
 */
struct NameChangeRequest {
    // The enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::NameChangeRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The entity to change the name of. */
    entt::entity entity{entt::null};

    /** The new name. */
    Name name;

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
void serialize(S& serializer, NameChangeRequest& nameChangeRequest)
{
    serializer.value4b(nameChangeRequest.entity);
    serializer.object(nameChangeRequest.name);
}

} // End namespace AM
