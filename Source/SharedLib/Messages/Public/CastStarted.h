#pragma once

#include "EngineMessageType.h"
#include "CastableID.h"
#include "Vector3.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"

namespace AM
{

/**
 * Sent by the server when a cast is started.
 */
struct CastStarted {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::CastStarted};

    /** The entity that is casting. */
    entt::entity casterEntity{entt::null};

    /** The castable that is being cast. */
    CastableID castableID{};

    // Note: We don't send the item ID because it doesn't seem like it'd ever 
    //       be useful (it's only useful to the server when handling the cast). 
    //       If a use case comes up for it, we can add it.

    /** The target entity. If castableID is an EntityInteractionType, this will
        always be present. Otherwise, this will be filled if the client has a 
        current target. */
    entt::entity targetEntity{entt::null};

    /** The target position. If the Castable has a targetToolType that selects 
        a position, this will be filled.*/
    Vector3 targetPosition{};
};

template<typename S>
void serialize(S& serializer, CastStarted& castStarted)
{
    serializer.value4b(castStarted.casterEntity);
    serializer.object(castStarted.castableID);
    serializer.value4b(castStarted.targetEntity);
    serializer.object(castStarted.targetPosition);
}

} // End namespace AM
