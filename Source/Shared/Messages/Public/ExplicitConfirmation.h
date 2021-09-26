#pragma once

#include <SDL2/SDL_stdinc.h>

/**
 * An explicit confirmation that ticks have passed.
 *
 * Sent from Server -> Client when a tick passes with no entity movement
 * updates. The client needs these confirmations before it can progress
 * NPC movement forward.
 */
struct ExplicitConfirmation
{
    Uint8 confirmedTickCount{0};
};

template<typename S>
void serialize(S& serializer, ExplicitConfirmation& explicitConfirmation)
{
    serializer.value1b(explicitConfirmation.confirmedTickCount);
}
