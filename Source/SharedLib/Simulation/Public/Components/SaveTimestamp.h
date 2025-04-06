#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * Tracks the tick number when this entity was last saved.
 *
 * An example of usage is our CastCooldown component. It lazily updates its 
 * timestamps, so if it gets saved on e.g. tick 500, it may have last been 
 * updated on tick 450. When we load the component, we need to update it to 
 * account for those 50 elapsed ticks. We can do that by using this timestamp.
 */
struct SaveTimestamp {
    /** The tick that this entity was last saved on. */
    Uint32 lastSavedTick{};
};

template<typename S>
void serialize(S& serializer, SaveTimestamp& saveTimestamp)
{
    serializer.value4b(saveTimestamp.lastSavedTick);
}

} // namespace AM
