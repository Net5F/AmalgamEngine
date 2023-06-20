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
    /** The world object that we want to modify the color of. */
    WorldObjectID objectToModify;

    /** The color and transparency to multiply the sprite by. */
    SDL_Color colorMod{255, 255, 255, 255};
};

} // End namespace Client
} // End namespace AM
