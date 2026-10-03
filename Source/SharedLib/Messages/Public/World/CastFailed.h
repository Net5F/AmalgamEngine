#pragma once

#include "EngineMessageType.h"
#include "CastableID.h"
#include "CastFailureType.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"

namespace AM
{

/**
 * Sent by the server when a cast either fails to validate, or is canceled.
 */
struct CastFailed {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::CastFailed};

    /** The entity that was casting. */
    entt::entity casterEntity{entt::null};

    /** The castable that was being cast. */
    CastableID castableID{};

    /** The reason why the cast failed. */
    CastFailureType castFailureType{};
};

template<typename S>
void serialize(S& serializer, CastFailed& castFailed)
{
    serializer.value4b(castFailed.casterEntity);
    serializer.object(castFailed.castableID);
    serializer.value1b(castFailed.castFailureType);
}

} // End namespace AM
