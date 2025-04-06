#pragma once

#include "EngineMessageType.h"
#include "CastCooldown.h"

namespace AM
{
/**
 * Sent by the server when a client needs their full list of cast cooldowns.
 * After the full list is sent, the client can update it locally as casts are 
 * completed.
 */
struct CastCooldownInit {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::CastCooldownInit};

    /**
     * The entity's full CastCooldown component.
     */
    CastCooldown castCooldown{};
};

template<typename S>
void serialize(S& serializer, CastCooldownInit& castCooldownInit)
{
    serializer.object(castCooldownInit);
}

} // namespace AM
