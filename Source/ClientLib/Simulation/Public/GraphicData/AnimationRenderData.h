#pragma once

#include "Vector3.h"
#include <optional>

namespace AM
{
namespace Client
{

/**
 * Holds a animation's rendering-related data.
 *
 * We store any rendering-only data separately, so the server can optimally 
 * access only the data it needs.
 *
 * See Animation.h for the rest of the animation data.
 */
struct AnimationRenderData {
    /** Only used for entities, during render sorting.
        When entities change animation, the new animation needs to line up with
        the old one so the entity doesn't look like it's teleporting around. 
        If non-null, this is the model-space point that should be aligned 
        with IdleSouth. */
    std::optional<Vector3> entityAlignmentAnchor{};
};

} // End namespace Client
} // End namespace AM
