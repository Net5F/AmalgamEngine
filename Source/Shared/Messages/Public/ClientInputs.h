#pragma once

#include "SDL_stdinc.h"
#include "InputComponent.h"

namespace AM
{
/**
 * This struct represents a client's input states on a given tick.
 */
struct ClientInputs {
    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** The tick that these client input states correspond to. */
    Uint32 tickNum = 0;

    /** The client's input state for the given tickNum. */
    InputComponent inputComponent;

    //--------------------------------------------------------------------------
    // Non-replicated data
    //--------------------------------------------------------------------------
    /**
     * The client's network ID, assigned by the server.
     * No IDs are accepted from the client because we can't trust it,
     * so we fill in the ID based on which socket the message came from.
     */
    Uint32 netID = 0;
};

template<typename S>
void serialize(S& serializer, ClientInputs& clientInputs)
{
    serializer.value4b(clientInputs.tickNum);
    serializer.object(clientInputs.inputComponent);
}

} // End namespace AM
