#pragma once

#include "EngineMessageType.h"
#include "Input.h"
#include "ConnectionHandle.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{
/**
 * Contains a client's input state on a given tick.
 *
 * Used by clients to request input changes on the server.
 */
struct InputChangeRequest {
    // The enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::InputChangeRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The tick that these client input states correspond to. */
    Uint32 tickNum{0};

    // Note: No entity ID, because clients can only change their own inputs.

    /** The client's input state for the given tickNum. */
    Input input;

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, InputChangeRequest& inputChangeRequest)
{
    serializer.value4b(inputChangeRequest.tickNum);

    serializer.enableBitPacking(
        [&inputChangeRequest](typename S::BPEnabledType& sbp) {
            sbp.object(inputChangeRequest.input);
        });
}

} // End namespace AM
