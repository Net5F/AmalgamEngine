#pragma once

#include "Vector3.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Holds movement-related entity data.
 *
 * X and Y-axis velocity is simulated with infinite friction while on the 
 * ground, and with no friction while in the air.
 * Z-axis velocity is simulated with a standard force of gravity.
 */
struct Movement {
    //--------------------------------------------------------------------------
    // Engine-Managed Variables
    //--------------------------------------------------------------------------
    /** The entity's current velocity, in world units per second.
        This is managed by the engine. Project devs should instead use 
        velocityMod. */
    Vector3 velocity{};

    // TODO: Figure out how we're gonna detect this, and a standard way to 
    //       do it in the 3 places where we resolve collisions.
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

    //--------------------------------------------------------------------------
    // Project-Managed Variables
    //--------------------------------------------------------------------------
    /** Velocity modifiers, to apply to the entity on the next tick.
        The project should change these when it wants to influence an entity's 
        movement. */
    Vector3 velocityMod{};

    /** The distance that the entity can travel per second, in world units.
        The project is responsible for managing this. The engine will never 
        change this value. */
    Uint16 runSpeed{48};

    /** The height of the entity's jump, from the bottom of the jump to the 
        peak height, in world units.
        The project is responsible for managing this. The engine will never 
        change this value. */
    Uint16 jumpHeight{48};

    /** The maximum number of times the entity can jump before needing to 
        touch the ground. */
    Uint8 maxJumpCount{1};

    /** If true, the entity will not be affected by gravity, and its jump and 
        crouch inputs will instead raise and lower it in the air. */
    bool canFly{false};
};

template<typename S>
void serialize(S& serializer, Movement& movement)
{
    serializer.object(movement.velocity);
    serializer.value1b(movement.canFly);
}

} // namespace AM
