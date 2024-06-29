#pragma once

#include <SDL_stdinc.h>

namespace AM
{
/**
 * Holds movement-related entity data.
 */
struct Movement {
    /** The distance that this entity is currently traveling in each direction,
        per second. */
    float velocityModX{0};
    float velocityModY{0};
    float velocityModZ{0};

    float velocityZ{0};

    /** The distance that this entity can travel per second, in world units.
        Used when the user presses the movement keys. */
    Uint16 runSpeed{16};

    /** The height of this entity's jump, from the bottom of the jump to the 
        peak height, in world units.
        Used when the user presses the jump key. */
    Uint16 jumpHeight{16};
};

} // namespace AM
