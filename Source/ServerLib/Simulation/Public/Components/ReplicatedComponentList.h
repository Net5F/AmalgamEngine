#pragma once

#include <SDL_stdinc.h>
#include <vector>

namespace AM
{
namespace Server
{
/**
 * Tracks which client-relevant components this entity currently has attached
 * to it.
 *
 * Client-relevant components are any that we need to replicate to the client,
 * e.g. Position. If the client doesn't care about it, it won't be tracked here.
 *
 * Note: This component will be present on an entity regardless of whether it
 *       has any replicated components or not. This is because the on_destroy()
 *       listener isn't allowed to remove components, so we just always leave
 * it.
 */
struct ReplicatedComponentList {
    /** Holds all currently attached component types that are relevant to
        the client.
        Each element refers to an index in the ReplicatedComponent variant. */
    std::vector<Uint8> typeIndices;
};

} // namespace Server
} // namespace AM
