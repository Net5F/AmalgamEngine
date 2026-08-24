#pragma once

#include "NetworkID.h"
#include <SDL3/SDL_stdinc.h>

namespace AM
{
/**
 * Stable local reference to a registered connection.
 *
 * The generation prevents delayed asynchronous work from addressing a newer
 * connection that happens to reuse the same NetworkID.
 */
struct ConnectionHandle {
    NetworkID networkID{NULL_NETWORK_ID};
    Uint64 generation{0};

    explicit operator bool() const
    {
        return (networkID != NULL_NETWORK_ID) && (generation != 0);
    }

    bool operator==(const ConnectionHandle&) const = default;
};

} // namespace AM
