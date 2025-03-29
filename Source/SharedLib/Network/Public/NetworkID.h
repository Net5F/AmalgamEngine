#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** A numeric ID used to locally identify a client socket.
    Note: This is only used locally by the server, but it lives in Shared 
          because it's convenient to use it in some shared code, e.g. message 
          definitions (so that we can use one struct on both sides). */
using NetworkID = Uint16;

/**
 * The ID used to indicate that a client is not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (networkID)".
 */
static constexpr NetworkID NULL_NETWORK_ID{0};

} // End namespace AM
