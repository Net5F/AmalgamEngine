#pragma once

#include "WorldObjectID.h"
#include <SDL_pixels.h>

namespace AM
{
namespace Client
{

/**
 * Used by the UI when it wants a sprite to have its color and/or transparency
 * modified.
 */
struct SpriteColorModInfo {
    /** The world object that we want to modify the color of.
        Note: Since phantom entities don't have an entity ID, you can use
              entt::null to target them. */
    WorldObjectID objectToModify;

    /** The color and transparency to multiply the sprite by.
        Note: Instead of directly multiplying the sprite by this color (which
              would make it darker), we render an additional sprite with an
              additive blend mode and multiply that one by this color.
              Alpha is applied to both sprites. */
    SDL_Color colorMod{0, 0, 0, 255};
};

} // End namespace Client
} // End namespace AM
