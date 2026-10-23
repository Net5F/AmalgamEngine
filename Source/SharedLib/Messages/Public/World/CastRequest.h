#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
#include "CastableID.h"
#include "Vector3.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Used to request that a castable be cast.
 */
struct CastRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::CastRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The tick that this cast corresponds to. */
    Uint32 tickNum{0};

    /** The castable that's being cast. */
    CastableID castableID{};

    /** If castableID is an ItemInteractionType, this is the inventory slot of
        the item that is being used. */
    Uint8 slotIndex{0};

    /** The target entity. If castableID is an EntityInteractionType, this
        must always be present. Otherwise, this should be filled if the client
        has a current target. */
    entt::entity targetEntity{entt::null};

    /** The target position. If the requested Castable has a targetToolType
        that selects a position, this must be filled. */
    Vector3 targetPosition{};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, CastRequest& castRequest)
{
    serializer.value4b(castRequest.tickNum);
    serializer.object(castRequest.castableID);
    serializer.value1b(castRequest.slotIndex);
    serializer.value4b(castRequest.targetEntity);
    serializer.object(castRequest.targetPosition);
}

} // End namespace AM
