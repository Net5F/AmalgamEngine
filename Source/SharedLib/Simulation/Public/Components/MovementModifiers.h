#pragma once

#include "Vector3.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Holds variables that affect an entity's movement.
 * 
 * The intended use of this is for projects to calculate all of their 
 * movement-related modifiers (gear bonuses, buffs/debuffs, knockbacks, etc), 
 * then to set these variables to reflect those total values. The project also 
 * must update these values as they change.
 *
 * The project is responsible for these values. The engine will never change 
 * them, only read them.
 *
 * TODO: Add a message for modifying this
 */
struct MovementModifiers {
    /** Velocity modifiers, to apply to the entity on the next tick. */
    Vector3 velocityMod{};

    /** The distance that the entity can travel per second, in world units. */
    Uint16 runSpeed{48};

    /** The vertical impulse added to the entity when jumping, in world units 
        per second. */
    Uint16 jumpImpulse{200};

    /** The maximum number of times the entity can jump before needing to 
        touch the ground. */
    Uint8 maxJumpCount{1};

    /** If true, the entity will not be affected by gravity, and its jump and 
        crouch inputs will instead raise and lower it in the air. */
    bool canFly{false};
};

template<typename S>
void serialize(S& serializer, MovementModifiers& movementMods)
{
    serializer.object(movementMods.velocityMod);
    serializer.value2b(movementMods.runSpeed);
    serializer.value2b(movementMods.jumpImpulse);
    serializer.value1b(movementMods.maxJumpCount);
    serializer.boolValue(movementMods.canFly);
}

} // namespace AM
