#pragma once

#include "EngineMessageType.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{

/**
 * Sent by the server to tell a client about the state of its logout.
 *
 * When the client receives Completed, its entity has already been removed
 * from the world, and it should disconnect.
 */
struct WorldLogoutUpdate {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::WorldLogoutUpdate};

    enum Type : Uint8 {
        /** The logout countdown was started. */
        Started,
        /** The logout countdown was canceled (or couldn't be started), e.g.
            because the entity moved. */
        Canceled,
        /** The logout countdown finished. The client's entity was removed. */
        Completed
    };

    /** The type of update. */
    Type type{Type::Started};

    /** If type == Started, how long the countdown will take, in seconds. */
    float delayS{0};
};

template<typename S>
void serialize(S& serializer, WorldLogoutUpdate& worldLogoutUpdate)
{
    serializer.value1b(worldLogoutUpdate.type);
    serializer.value4b(worldLogoutUpdate.delayS);
}

} // End namespace AM
