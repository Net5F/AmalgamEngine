#pragma once

#include "Vector3.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Holds an entity's current movement state, calculated each tick by the 
 * relevant movement system.
 *
 * X and Y-axis velocity is simulated with infinite friction while on the 
 * ground, and with no friction while in the air.
 * Z-axis velocity is simulated with a standard force of gravity.
 *
 * All non-velocity variables in this component primarily exist (along with the 
 * Input and MovementStats components) to contribute to the calculation of 
 * velocity.
 */
struct Movement {
    /** The entity's current velocity, in world units per second.
        This is managed by the engine. Project devs should instead use 
        velocityMod. */
    Vector3 velocity{};

    /** If false, the entity is currently standing on top of something.
        If true, the entity is falling through the air. */
    bool isFalling{false};

    /** The number of times the entity has jumped since last touching the 
        ground. */
    Uint8 jumpCount{0};

    /** If true, the jump input has already been processed and is being held.
        Used to ensure the input is released and re-pressed, to prevent 
        accidental air jumps. */
    bool jumpHeld{false};
};

template<typename S>
void serialize(S& serializer, Movement& movement)
{
    serializer.object(movement.velocity);
    serializer.value1b(movement.jumpCount);

    // Note: Packing this field is necessary, otherwise it wouldn't match 
    //       MeasureSize (which always has bit packing enabled).
    serializer.enableBitPacking(
        [&movement](typename S::BPEnabledType& sbp) {
            sbp.boolValue(movement.isFalling);
            sbp.boolValue(movement.jumpHeld);
        });

    // Align after bit-packing to make sure the following bytes can be easily
    // processed.
    serializer.adapter().align();
}

} // namespace AM
