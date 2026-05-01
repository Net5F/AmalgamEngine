#pragma once

#include <SDL3/SDL_stdinc.h>
#include <vector>

namespace AM
{
namespace Server
{
/**
 * Tracks which InRangeInit components this entity currently has attached to it.
 * See EngineReplicatedComponentTypes.h for more info on InRangeInit components.
 * 
 * Used by ClientAOISystem to fill EntityInit messages.
 *
 * Note: This component will be present on an entity regardless of whether it
 *       has any replicated components or not. This is because the on_destroy()
 *       listener isn't allowed to remove components, so we just always leave
 *       it.
 */
struct InRangeInitComponentList {
    /** Holds all currently attached InRangeInit component types.
        Each element refers to an index in the ReplicatedComponent variant. */
    std::vector<Uint8> typeIndices{};
};

} // namespace Server
} // namespace AM
