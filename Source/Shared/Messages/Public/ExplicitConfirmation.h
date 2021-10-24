#pragma once

#include "MessageType.h"
#include <SDL2/SDL_stdinc.h>

namespace AM
{
/**
 * An explicit confirmation that ticks have passed.
 *
 * Sent from Server -> Client when a tick passes with no entity movement
 * updates. The client needs these confirmations before it can progress
 * NPC movement forward.
 */
struct ExplicitConfirmation {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE
        = MessageType::ExplicitConfirmation;

    Uint8 confirmedTickCount{0};
};

template<typename S>
void serialize(S& serializer, ExplicitConfirmation& explicitConfirmation)
{
    serializer.value1b(explicitConfirmation.confirmedTickCount);
}

} // End namespace AM
