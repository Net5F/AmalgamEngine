#pragma once

#include "SDL_stdinc.h"
#include "Input.h"

namespace AM
{
/**
 * Contains a client's input state on a given tick.
 *
 * Used by clients for requesting input changes on the server.
 */
struct ClientInput {
    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** The tick that these client input states correspond to. */
    Uint32 tickNum{0};

    /** The client's input state for the given tickNum. */
    Input input;

    //--------------------------------------------------------------------------
    // Non-replicated data
    //--------------------------------------------------------------------------
    /**
     * The client's network ID, assigned by the server.
     * No IDs are accepted from the client because we can't trust it,
     * so we fill in the ID based on which socket the message came from.
     */
    Uint32 netID{0};
};

template<typename S>
void serialize(S& serializer, ClientInput& clientInputs)
{
    serializer.value4b(clientInputs.tickNum);
    serializer.object(clientInputs.input);
}

} // End namespace AM
