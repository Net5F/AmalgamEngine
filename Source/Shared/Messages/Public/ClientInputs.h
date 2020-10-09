#pragma once

#include "SDL_stdinc.h"
#include "InputComponent.h"

namespace AM
{
/**
 * This struct represents a client's input states on a given tick.
 */
struct ClientInputs {
    /** This entity's sim ID. */
    Uint32 id = 0;

    /** The tick that these client input states correspond to. */
    Uint32 tickNum = 0;

    InputComponent inputComponent;
};

template<typename S>
void serialize(S& serializer, ClientInputs& clientInputs)
{
    serializer.value4b(clientInputs.id);
    serializer.value4b(clientInputs.tickNum);
    serializer.object(clientInputs.inputComponent);
}

} // End namespace AM
